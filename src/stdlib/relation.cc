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
// along with pyRASMUS.  If not, see <http://www.gnu.org/licensgges/>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <set>
#include <stdlib/refptr.hh>
#include <stdlib/text.hh>
#include <stdlib/relation.hh>
#include <stdlib/callback.hh>
#include <stdlib/anyvalue.hh>
#include <stdlib/ile.hh>

namespace {
using namespace rasmus::stdlib;

int rm_itemWidth(AnyValue av){
	switch(av.type){
	case TBool:
		if(av.boolValue == RM_NULLBOOL) return 6; // strlen("?-Bool")
		else return 5; // strlen("false")
	case TInt:
		if(av.intValue == RM_NULLINT) return 5; // strlen("?-Int")
		else return std::to_string(av.intValue).size();
	case TText:
		return av.objectValue.getAs<TextBase>()->length;
	default:
		ILE("Unhandled type");
	}
}

} // nameless namespace

namespace rasmus {
namespace stdlib {

void printRelationToStream(rm_object * ptr, std::ostream & out) {
	Relation * relation = static_cast<Relation *>(ptr);
	Schema * schema = relation->schema.get();

	// handle special case where number of columns is zero
	const char * EMPTY_MSG = "(empty)";
	Schema alt_schema;
	bool empty_schema = schema->attributes.size() == 0;
	if(empty_schema){
		Attribute alt_attribute {
			TText, EMPTY_MSG
		};
		alt_schema.attributes.push_back(std::move(alt_attribute));
		schema = &alt_schema;
	}

	// calculate widths
	size_t num_attributes = schema->attributes.size();
	std::vector<int> widths(num_attributes, 0);

	// widths for tuples
	for(auto & tuple : relation->tuples){
		int i = 0;		
		for(auto & item : tuple->values){
			widths[i] = std::max(widths[i], rm_itemWidth(item));
			i++;
		}
	}

	// widths for schema names
	int i = 0;
	for(auto & attribute : schema->attributes){
		widths[i] = std::max<int>(widths[i], attribute.name.size());
		i++;
	}
	
	size_t total_width = std::accumulate(begin(widths), end(widths), 0);
	total_width += 1 + widths.size() * 3; 
    // 1 for beginning |, 3 per field for ' | ' (space, bar, space)

	// print header
	out << std::string(total_width, '-') << std::endl;
	out << "|";

	i = 0;
	for(auto & attribute : schema->attributes){
		out << ' ' << std::left << std::setw(widths[i]) << attribute.name <<  " |";
		i++;
	}
	
	out << std::endl;

	// print body
	out << std::string(total_width, '-') << std::endl;

	if(empty_schema){
		for(size_t i = 0; i < relation->tuples.size(); i++)
			out << "| " << EMPTY_MSG << " |" << std::endl;

	} else {
		for(auto & tuple : relation->tuples){
			i = 0;
			out << '|';
			for(auto & value : tuple->values){
				switch(value.type){
				case TInt:
					out << ' ' << std::right << std::setw(widths[i]);
					printIntToStream(value.intValue, out);
					break;
				case TBool:
					out << ' ' << std::left << std::setw(widths[i]);
					printBoolToStream(value.boolValue, out);
					break;
				case TText:
					out << ' ' << std::left << std::setw(widths[i]);
					out << textToString(value.objectValue.getAs<TextBase>());
					break;
				default:
					ILE("Unhandled type", value.type);
				}
				out << " |";
				i++;
			}
			out << std::endl;
		}
	}

	out << std::string(total_width, '-') << std::endl;
} 

/*  outputs the given relation to the given stream. 

	The output format is as given in the RASMUS 
	user manual:

	First the size of the schema is output, then
	the type and name of each attribute, and finally
	the values of the tuples in the relation.
 */
void saveRelationToStream(rm_object * o, std::ostream & outFile){
	Relation * relation = static_cast<Relation *>(o);
	outFile << relation->schema->attributes.size() << std::endl;
	for(auto & attribute : relation->schema->attributes){
		switch(attribute.type){
		case TInt:
			outFile << "I ";
			break;
		case TBool:
			outFile << "B ";
			break;
		case TText:
			outFile << "T ";
			break;
		default:
			ILE("Unhandled type", attribute.type);
		}
		outFile << attribute.name << std::endl;
		
	}

	for(auto & tuple : relation->tuples){
		for(auto & value : tuple->values){
			switch(value.type){
			case TInt:
				outFile << value.intValue << std::endl;
				break;
			case TBool:
				outFile << (value.boolValue ? "true" : "false") << std::endl;
				break;
			case TText:
				printTextToStream(value.objectValue.getAs<TextBase>(), outFile);
				outFile << std::endl;
				break;
			default:
				ILE("Unhandled type", value.type);
			}
		}
	}
	
}	

/*  parses a relation given by an input stream and returns 
	it. The format is as given in the RASMUS user manual. 

	The attribute types and names are saved in relations->schema
	and relations->tuples contains the tuples for the relation
*/
rm_object * loadRelationFromStream(std::istream & inFile){

	size_t num_columns;
	RefPtr<Schema> schema;

	if(!(inFile >> num_columns)){
		std::cerr << "could not read number of attributes from file "  << std::endl;
		exit(EXIT_FAILURE);
	}

	schema = makeRef<Schema>();
	
	for(size_t i = 0; i < num_columns; i++){
		char type;
		std::string name;
		Attribute attribute;

		if(!(inFile >> type >> name)){
			std::cerr << "wrong schema format in file "  << std::endl;
			exit(EXIT_FAILURE);
		}

		attribute.name = std::move(name);

		switch(type){
		case 'T':
			attribute.type = TText;
			break;
		case 'I':
			attribute.type = TInt;
			break;
		case 'B':
			attribute.type = TBool;
			break;
		default:
			std::cerr << "wrong schema format in file "  << std::endl;
			exit(EXIT_FAILURE);			
		}

		schema->attributes.push_back(std::move(attribute));

	}

	{
		std::string _;
		getline(inFile, _);
	}

	RefPtr<Relation> relations = makeRef<Relation>();
	relations->schema = schema;

	bool done = false;

	while(!done){
		
		RefPtr<Tuple> tuple = makeRef<Tuple>();
		tuple->schema = schema;

		for(auto & attribute : schema->attributes){
			
			std::string line;
			if(!getline(inFile, line)){
				done = true;
				break;
			}

			switch(attribute.type){
			case TInt:
			{
				std::stringstream ss(line);
				int64_t value;
				ss >> value;
				tuple->values.emplace_back(value);
				break;
			}
			case TBool:
				tuple->values.emplace_back((line != "false") ? RM_TRUE : RM_FALSE);
				break;
			case TText:
				tuple->values.emplace_back(TText, RefPtr<rm_object>::steal(rm_getConstText(line.c_str())));
				break;
			default:
				std::cerr << "internal library error" << std::endl;
				exit(EXIT_FAILURE);			
				break;
			}
			
		}

		if(done) break;
		
		relations->tuples.push_back(std::move(tuple));

	}
	
	return relations.unbox();
}

void saveRelationToFile(rm_object * rel, const char * name) {
	std::ofstream file(name);
	saveRelationToStream(rel, file);
}

rm_object * loadRelationFromFile(const char * name) {
	std::ifstream file(name);
	return loadRelationFromStream(file);
}

/**
 * \Brief Prints an integer to a stream
 * Handles the special case where the integer is ?-Int
 */
void printIntToStream(int64_t val, std::ostream & out){
	if(val == RM_NULLINT)
		out << "?-Int";
	else
		out << val;
}

/**
 * \Brief Prints a boolean to a stream
 * Handles the special case where the boolean is ?-Bool
 */
void printBoolToStream(int8_t val, std::ostream & out){
	switch (val) {
	case RM_TRUE: out << "true"; break;
	case RM_FALSE: out << "false"; break;
	default: out << "?-Bool"; break;
	}
}

/**
 * \Brief prints the given tuple to the out stream
 */
void printTupleToStream(rm_object * ptr, std::ostream & out) {
	Tuple * tup = static_cast<Tuple *>(ptr);
	Schema * schema = tup->schema.get();

	out << "(";
	
	for(size_t i = 0; i < schema->attributes.size(); i++){
		out << schema->attributes[i].name;
		out << ": ";
		switch(schema->attributes[i].type){
			case TInt:
				printIntToStream(tup->values[i].intValue, out);
				break;
			case TBool:
				printBoolToStream(tup->values[i].boolValue, out);
				break;
			case TText:
				printQuoteTextToStream(tup->values[i].objectValue.getAs<TextBase>(), out);
				break;
			default:
				ILE("Unsupported type", schema->attributes[i].type);
		}
		if(i != schema->attributes.size() - 1) 
			out << ", ";
	}

	out << ")";

}

/**
 * \Brief find the index of the column with name 'name'
 */
size_t getColumnIndex(Relation * rel, const char * name){
	size_t i;
	for(i = 0; i < rel->schema->attributes.size(); i++)
		if(name == rel->schema->attributes[i].name)
			break;

	if(i == rel->schema->attributes.size())
		ILE("A column with name", name, "does not exist in the given relation");

	return i;
}

/**
 * \Brief Removes duplicate rows in a relation
 * This is done by sorting the relation's tuples, and then doing
 * a linear sweep through them to remove any duplicates
 */
void removeDuplicateRows(Relation * rel){
	std::sort(rel->tuples.begin(), rel->tuples.end(),
			  [](const RefPtr<Tuple> & l,
				 const RefPtr<Tuple> & r)->bool{
				  return *l < *r;
			  });

	rel->tuples.erase(
		std::unique(
			rel->tuples.begin(), 
			rel->tuples.end(),
			[](const RefPtr<Tuple> & l,
			   const RefPtr<Tuple> & r)->bool{
				return *l == *r;
			}),
		rel->tuples.end());
}

/**
 * \Brief Projects a relation onto the columns numbered in 'indices'
 */
RefPtr<Relation> projectByIndices(Relation * rel, std::vector<size_t> indices){

	RefPtr<Relation> ret = makeRef<Relation>();
	RefPtr<Schema> schema = makeRef<Schema>();

	for(size_t index : indices)
		schema->attributes.push_back(rel->schema->attributes[index]);
	ret->schema = schema;

	// create new tuples and add them to the new relation
	for(auto old_tuple : rel->tuples){
		RefPtr<Tuple> new_tuple = makeRef<Tuple>();
		new_tuple->schema = schema;
		for(size_t index : indices)
			new_tuple->values.push_back(old_tuple->values[index]);
		ret->tuples.push_back(std::move(new_tuple));
	}

	return ret;
}

bool cmpTupPtrs(const RefPtr<Tuple> l, const RefPtr<Tuple> r){
	return *l < *r;
}

/**
 * \Brief Restricts a tuple according to the values in 'indices'
 * \Note The schema of the resulting tuple is not set for efficiency reasons
 */
Tuple restrict(const RefPtr<Tuple> & orig, const std::vector<size_t> indices){
	Tuple ret;
	for(size_t index : indices)
		ret.values.push_back(orig->values[index]);
	
	return ret;
}

/**
 * \Brief Returns the tuple which is a union of 'left' and 'right'.
 * lsi and rsi contain the indices of the columns in the left and 
 * right tuple which are shared
 */
RefPtr<Tuple> rowUnion(RefPtr<Tuple> left, RefPtr<Tuple> right, 
					   std::vector<size_t> lsi, std::vector<size_t> rsi){

	// make 'ret' a copy of 'left' and expand it later
	RefPtr<Tuple> ret = makeRef<Tuple>();
	RefPtr<Schema> schema = makeRef<Schema>();
	schema->attributes = left->schema->attributes;
	ret->schema = schema;
	ret->values = left->values;

	// no columns from rsi should be added; they would be duplicates
	std::vector<bool> add_column(right->values.size(), 1);
	for(size_t col : rsi)
		add_column[col] = 0;

	// add values and names from 'right' to 'ret'
	for(size_t i = 0; i < right->values.size(); i++){
		if(add_column[i]){
			ret->schema->attributes.push_back(right->schema->attributes[i]);
			ret->values.push_back(right->values[i]);
		}
	}
	
	return ret;
}


/**
 * This function makes it possible to run the special function
 * given as an argument to rm_factorRel. It will run the function,
 * giving it 'tup' as a first argument, and each relation in 'relations'
 * as the following arguments
 * \Note this code is autogenerated; a maximum of 16 arguments to run() is currently supported
 */
void * run(void * base, void * ret, void * func, int64_t tup, std::vector<int64_t> relations) {
	typedef void * (*t0)(void *, void *, int64_t, int8_t);
	typedef void * (*t1)(void *, void *, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t2)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t3)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t4)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t5)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t6)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t7)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t8)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t9)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t10)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t11)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t12)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t13)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t14)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t15)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	switch(relations.size()) {
	case 0: return ((t0) func)(base, ret, tup, TTup);
	case 1: return ((t1) func)(base, ret, tup, TTup, relations[0], TRel);
	case 2: return ((t2) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel);
	case 3: return ((t3) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel);
	case 4: return ((t4) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel);
	case 5: return ((t5) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel);
	case 6: return ((t6) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel);
	case 7: return ((t7) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel);
	case 8: return ((t8) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel);
	case 9: return ((t9) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel);
	case 10: return ((t10) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel);
	case 11: return ((t11) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel);
	case 12: return ((t12) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel);
	case 13: return ((t13) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel);
	case 14: return ((t14) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel);
	case 15: return ((t15) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel);
	default: ILE("run() was called with too many arguments");
	}
}


bool schemaEquals(Schema * l, Schema * r){
	if(l->attributes.size() != r->attributes.size()) 
		return false;

	std::vector<std::pair<std::string, size_t>> l_indices;
	std::vector<std::pair<std::string, size_t>> r_indices;
	
	for(size_t i = 0; i < l->attributes.size(); i++){
		l_indices.emplace_back(l->attributes[i].name, i);
		r_indices.emplace_back(r->attributes[i].name, i);
	}

	std::sort(l_indices.begin(), l_indices.end());
	std::sort(r_indices.begin(), r_indices.end());

	for(size_t i = 0; i < l->attributes.size(); i++){
		size_t left_index = l_indices[i].second;
		size_t right_index = r_indices[i].second;
		if(l->attributes[left_index].name != 
		   r->attributes[right_index].name || 
		   l->attributes[left_index].type !=
		   r->attributes[right_index].type)
			return false;
	}

	return true;

}


} //namespace stdlib
} //namespace rasmus

extern "C" {

//TODO fix me
Relation zero_rel;
//TODO fix me
Relation one_rel;


class __init{
public:
	__init(){
		RefPtr<Schema> zero_schema = makeRef<Schema>();
		zero_rel.schema = zero_schema;
		one_rel.schema = zero_schema;

		RefPtr<Tuple> empty_tuple = makeRef<Tuple>();
		empty_tuple->schema = zero_schema;
		one_rel.tuples.push_back(std::move(empty_tuple));

		zero_rel.ref_cnt = 1;
		one_rel.ref_cnt = 1;
	}
};
__init foo;

/* wrapper function */
void rm_saveRel(rm_object * o, const char * name) {
	callback->saveRelation(o, name);
}

/* wrapper function */
rm_object * rm_loadRel(const char * name) {
	return callback->loadRelation(name);
}

/**
 * \Brief Perform a natural join on lhs and rhs

 * First we define a shared schema as all columns which
 * are in both lhs and rhs

 * Then we define the restriction of a tuple as 
 * projecting the tuple onto the shared schema
 * For example, we define lhs := tup(a: 1, b: 2)
 * and rhs:= tup(b: 2, c: 3) then the shared schema
 * is (b) and the restriction of both lhs and rhs 
 * is (b: 2)

 * Two restricted tuples are compared just as normal tuples

 * Now we are ready to describe the algorithm

 * First we compute the shared schema

 * Then we sort lhs and rhs individually on the 
 * restriction of their tuples (so row 1 in lhs is
 * now smaller than any other row, with respect to its
 * restriction, etc.)

 * We maintain two pointers i and j pointing to rows
 * in lhs and rhs, respectively
 * While we are not at the end of both relations, 
 * we do the following
 
 * We find the smallest restriction of a pointed-to tuple
 * Then we find the interval of rows in lhs and rhs whose
 * restricted tuples are equal to the current one. For all possible
 * combinations of rows lhs and rhs which are in the interval, we add
 * their union to the result-relation

 */
rm_object * rm_joinRel(rm_object * lhs, rm_object * rhs) {

	Relation * l = static_cast<Relation *>(lhs);
	Relation * r = static_cast<Relation *>(rhs);

	// compute the shared schema
	std::vector<std::pair<std::string, size_t>> l_indices;
	std::vector<std::pair<std::string, size_t>> r_indices;
	
	for(size_t i = 0; i < l->schema->attributes.size(); i++)
		l_indices.emplace_back(l->schema->attributes[i].name, i);
	for(size_t j = 0; j < r->schema->attributes.size(); j++)
		r_indices.emplace_back(r->schema->attributes[j].name, j);

	std::sort(l_indices.begin(), l_indices.end());
	std::sort(r_indices.begin(), r_indices.end());

	// left and right shared-schema indices
	// these will be used to restrict tuples
	std::vector<size_t> lsi; 
	std::vector<size_t> rsi;

	for(size_t i = 0, j = 0; i < l->schema->attributes.size()
			&& j < r->schema->attributes.size(); ){
		size_t l_index = l_indices[i].second;
		size_t r_index = r_indices[j].second;

		if(l->schema->attributes[l_index].name ==
		   r->schema->attributes[r_index].name){
			if(l->schema->attributes[l_index].type !=
			   r->schema->attributes[r_index].type)
				// TODO Q is this the right way to handle a type collision?
				ILE("The column", l->schema->attributes[l_index].name, "has different types in each relation!");
			lsi.push_back(l_index);
			rsi.push_back(r_index);
			i++;
			j++;
		} else if (l->schema->attributes[l_index].name <
				   r->schema->attributes[r_index].name)
			i++;
		else
			j++;
	}

	// sort lhs and rhs, respectively, by their restricted tuples
	std::sort(l->tuples.begin(), l->tuples.end(),
			  [&](const RefPtr<Tuple> & a,
				  const RefPtr<Tuple> & b)->bool{
				  return restrict(a, lsi) <
					  restrict(b, lsi);
			  });
	
	std::sort(r->tuples.begin(), r->tuples.end(),
			  [&](const RefPtr<Tuple> & a,
				  const RefPtr<Tuple> & b)->bool{
				  return restrict(a, rsi) <
					  restrict(b, rsi);
			  });

	// create the relation to be returned and compute its schema
	RefPtr<Relation> ret = makeRef<Relation>();
	RefPtr<Schema> schema = makeRef<Schema>();
	schema->attributes = l->schema->attributes;
	ret->schema = schema;

	// no columns from rsi should be added; they would be duplicates
	std::vector<bool> add_column(r->schema->attributes.size(), 1);
	for(size_t col : rsi)
		add_column[col] = 0;
	for(size_t i = 0; i < r->schema->attributes.size(); i++)
		if(add_column[i])
			ret->schema->attributes.push_back(r->schema->attributes[i]);

	// perform the natural join on lhs and rhs, adding tuples to 'ret'
	for(size_t l_start = 0, r_start = 0; 
		l_start < l->tuples.size() && r_start < r->tuples.size(); ){
		
		// find the smallest restriction of a pointed-to tuple
		Tuple smallest = std::min(restrict(l->tuples[l_start], lsi), 
								  restrict(r->tuples[r_start], rsi));

		// find the intervals of rows with restricted rows
		// equal to the smallest one
		size_t l_end = l_start, r_end = r_start;
		while(l_end < l->tuples.size() && restrict(l->tuples[l_end], lsi) == smallest) 
			l_end++;
		while(r_end < r->tuples.size() && restrict(r->tuples[r_end], rsi) == smallest) 
			r_end++;

		// for all combinations of rows in the interval,
		// add their union to the result
		for(size_t i = l_start; i < l_end; i++){
			for(size_t j = r_start; j < r_end; j++){
				RefPtr<Tuple> tup = rowUnion(l->tuples[i], r->tuples[j], lsi, rsi);
				ret->tuples.push_back(std::move(tup));
			}
		}

		l_start = l_end;
		r_start = r_end;

	}

	return ret.unbox();
}


/**
 * \Brief return the union of lhs and rhs
 * First check if lhs and rhs have the same schema
 * If not, throw an error
 * Otherwise return the union of the two relations
 */
rm_object * rm_unionRel(rm_object * lhs, rm_object * rhs) {

	Relation * l = static_cast<Relation *>(lhs);
	Relation * r = static_cast<Relation *>(rhs);

	// ensure the left relation is never the smaller one
	// this will make things more efficient later
	if(l->tuples.size() < r->tuples.size()){
		Relation * tmp = l;
		l = r;
		r = tmp;
	}
	
	// ensure schema equality
	if(l->schema->attributes.size() != r->schema->attributes.size()) 
		ILE("The given schemas are not equal!");

	std::vector<std::pair<std::string, size_t>> l_indices;
	std::vector<std::pair<std::string, size_t>> r_indices;
	
	for(size_t i = 0; i < l->schema->attributes.size(); i++){
		l_indices.emplace_back(l->schema->attributes[i].name, i);
		r_indices.emplace_back(r->schema->attributes[i].name, i);
	}

	std::sort(l_indices.begin(), l_indices.end());
	std::sort(r_indices.begin(), r_indices.end());

	for(size_t i = 0; i < l->schema->attributes.size(); i++){
		size_t left_index = l_indices[i].second;
		size_t right_index = r_indices[i].second;
		if(l->schema->attributes[left_index].name != 
		   r->schema->attributes[right_index].name || 
		   l->schema->attributes[left_index].type !=
		   r->schema->attributes[right_index].type)
			ILE("The given schemas are not equal!");
	}
	
	// build a new relation from lhs
	RefPtr<Relation> rel = makeRef<Relation>();

	rel->schema = l->schema;
	rel->tuples = l->tuples;

	// add all entries from rhs to the relation
	for(auto & old_tup : r->tuples){
		RefPtr<Tuple> new_tup = makeRef<Tuple>();
		new_tup->schema = rel->schema;
		new_tup->values.resize(l->schema->attributes.size());
		for(size_t i = 0; i < l->schema->attributes.size(); i++){
			size_t key_index = l_indices[i].second;
			size_t val_index = r_indices[i].second;
			AnyValue val = old_tup->values[val_index];
			new_tup->values[key_index] = val;
		}
		rel->tuples.push_back(std::move(new_tup));
	}

	removeDuplicateRows(rel.get());
	
	return rel.unbox();
}

/**
 * \Brief Return the difference of lhs and rhs
 * This is done by first asserting schema equality.
 * Then two sets are created, and their difference
 * is returned.
 */
rm_object * rm_diffRel(rm_object * lhs, rm_object * rhs) {

	Relation * l = static_cast<Relation *>(lhs);
	Relation * r = static_cast<Relation *>(rhs);

	// ensure schema equality
	if(l->schema->attributes.size() != r->schema->attributes.size()) 
		ILE("The given schemas are not equal!");

	std::vector<std::pair<std::string, size_t>> l_indices;
	std::vector<std::pair<std::string, size_t>> r_indices;
	
	for(size_t i = 0; i < l->schema->attributes.size(); i++){
		l_indices.emplace_back(l->schema->attributes[i].name, i);
		r_indices.emplace_back(r->schema->attributes[i].name, i);
	}

	std::sort(l_indices.begin(), l_indices.end());
	std::sort(r_indices.begin(), r_indices.end());

	RefPtr<Schema> common_schema = makeRef<Schema>();

	for(size_t i = 0; i < l->schema->attributes.size(); i++){
		size_t left_index = l_indices[i].second;
		size_t right_index = r_indices[i].second;
		if(l->schema->attributes[left_index].name != 
		   r->schema->attributes[right_index].name || 
		   l->schema->attributes[left_index].type !=
		   r->schema->attributes[right_index].type)
			ILE("The given schemas are not equal!");
		common_schema->attributes.push_back(l->schema->attributes[left_index]);
	}

	// create new vectors containing tuples, all of which have identical schemas
	std::vector< RefPtr<Tuple> > ls;
	std::vector< RefPtr<Tuple> > rs;
	
	for(auto & tuple : l->tuples){
		RefPtr<Tuple> new_tup = makeRef<Tuple>();
		new_tup->schema = common_schema;
		for(size_t j = 0; j < common_schema->attributes.size(); j++){
			size_t index = l_indices[j].second;
			new_tup->values.push_back(tuple->values[index]);
		}
		ls.push_back(std::move(new_tup));
	}

	for(auto & tuple : r->tuples){
		RefPtr<Tuple> new_tup = makeRef<Tuple>();
		new_tup->schema = common_schema;
		for(size_t j = 0; j < common_schema->attributes.size(); j++){
			size_t index = r_indices[j].second;
			new_tup->values.push_back(tuple->values[index]);
		}
		rs.push_back(std::move(new_tup));
	}

	std::sort(ls.begin(), ls.end(),
			  [](const RefPtr<Tuple> & l,
				 const RefPtr<Tuple> & r)->bool{
				  return *l < *r;
			  });
	std::sort(rs.begin(), rs.end(),
			  [](const RefPtr<Tuple> & l,
				 const RefPtr<Tuple> & r)->bool{
				  return *l < *r;
			  });

	RefPtr<Relation> ret = makeRef<Relation>();
	ret->schema = common_schema;

	std::set_difference(ls.begin(), ls.end(), rs.begin(), rs.end(),
						std::inserter(ret->tuples, ret->tuples.end()),
						cmpTupPtrs);

	return ret.unbox();
}

struct FuncBase : rm_object {
	// from rm_object:
	// uint32_t refcnt;
	// uint16_t type;
	uint16_t args;
	void * dtor;
	void * func;
	// captured variables here
};

typedef	void (* selectFunc) (FuncBase *, AnyRet *, int64_t, int8_t);


/**
 * Evaluate func on every tuple in rel
 * If this evaluates to RM_TRUE, the tuple is included in the result
 */
rm_object * rm_selectRel(rm_object * rel_, rm_object * func) {

	Relation * rel = static_cast<Relation *>(rel_);
	FuncBase * base = (FuncBase *) func;
	selectFunc f = (selectFunc) base->func;
	AnyRet retval;

	RefPtr<Relation> ret = makeRef<Relation>();
	RefPtr<Schema> schema = makeRef<Schema>();
	schema->attributes = rel->schema->attributes;
	ret->schema = schema;

	for(size_t i = 0; i < rel->tuples.size(); i++){
		f(base, &retval, reinterpret_cast<int64_t>(rel->tuples[i].get()), TTup);
		if(retval.value == RM_TRUE)
			ret->tuples.push_back(rel->tuples[i]);
	}
	
	return ret.unbox();
}

/**
 * \Brief Projects rel onto the given set of names
 * Iterate over all tuples, replacing them with new tuples
 * according to the given list of names
 * \Note duplicates must be removed
 */
rm_object * rm_projectPlusRel(rm_object * rel_, uint32_t name_count, const char ** names) {
	
	Relation * rel = static_cast<Relation *>(rel_);

	// find the indices of the names we want to keep
	// by creating a sorted vector of the input names
	// and a sorted vector of pairs of schema names and indices.
	// Then do a linear sweep and add the matching indices to a new vector
	std::vector<std::string> inputNames;
	std::vector<std::pair<std::string, size_t>> schemaNames;
	for(uint32_t i = 0; i < name_count; i++)
		inputNames.push_back(names[i]);
	for(size_t i = 0; i < rel->schema->attributes.size(); i++)
		schemaNames.emplace_back(rel->schema->attributes[i].name, i);
	std::sort(inputNames.begin(), inputNames.end());
	std::sort(schemaNames.begin(), schemaNames.end());

	std::vector<size_t> indices;
	if(inputNames.size() > schemaNames.size())
		ILE("The relation does not have that many columns!");

	for(size_t i = 0, j = 0; i < schemaNames.size() && j < inputNames.size(); i++){
		if(schemaNames[i].first == inputNames[j]){
			indices.push_back(schemaNames[i].second);
			j++;
		}
	}
	if(indices.size() != inputNames.size())
		ILE("The relation does not contain all the given columns");

	// project the relation onto the found indices
	RefPtr<Relation> ret = projectByIndices(rel, indices);
	removeDuplicateRows(ret.get());

	return ret.unbox();

}

/** \Brief Projects rel into all names except the given set of names
 */
rm_object * rm_projectMinusRel(rm_object * rel_, uint32_t name_count, const char ** names) {

	Relation * rel = static_cast<Relation *>(rel_);

	// find the indices of the names we want to keep
	std::vector<std::string> inputNames;
	std::vector<std::pair<std::string, size_t>> schemaNames;
	for(uint32_t i = 0; i < name_count; i++)
		inputNames.push_back(names[i]);
	for(size_t i = 0; i < rel->schema->attributes.size(); i++)
		schemaNames.emplace_back(rel->schema->attributes[i].name, i);
	std::sort(inputNames.begin(), inputNames.end());
	std::sort(schemaNames.begin(), schemaNames.end());

	std::vector<size_t> indices;

	for(size_t i = 0, j = 0; i < schemaNames.size(); i++){
		if(j < inputNames.size() && schemaNames[i].first == inputNames[j])
			j++;
		else
			indices.push_back(schemaNames[i].second);
	}

	// project the relation onto the found indices
	RefPtr<Relation> ret = projectByIndices(rel, indices);
	removeDuplicateRows(ret.get());

	return ret.unbox();
}

/**
 * \Brief Replace the names in the relation with a set of replacement names
 * \Note "names" has twice the length of name_count; it holds both the old names and the new ones
 */
rm_object * rm_renameRel(rm_object * rel, uint32_t name_count, const char ** names) {

	Relation * old_rel = static_cast<Relation *>(rel);

	RefPtr<Relation> relation = makeRef<Relation>();
		
	for(auto & old_tup : old_rel->tuples){
		RefPtr<Tuple> new_tup = makeRef<Tuple>();
		new_tup->values = old_tup->values;
		// tuple's schema will be updated later
		relation->tuples.push_back(std::move(new_tup));
	}

	// construct a new schema, copying the old one
	RefPtr<Schema> schema = makeRef<Schema>();
	relation->schema = schema;

	schema->attributes = old_rel->schema->attributes;

	if(schema->attributes.size() < name_count)
		ILE("Attempt to change", name_count, "names, but the relation only contains", schema->attributes.size(), "names");

	for(uint32_t i = 0; i < name_count; i++){
		std::string old_name = names[i*2];
		std::string new_name = names[i*2 + 1];
		bool found = false;
		for(auto & attribute : schema->attributes){
			if(attribute.name == old_name){
				attribute.name = new_name;
				found = true;
				break;
			}
		}
		if(!found) ILE("Schema has no such name", old_name);
	}

	// check if we have produced a schema with duplicate names
	std::set<std::string> schema_names;
	for(auto attribute : schema->attributes)
		schema_names.insert(attribute.name);
	if(schema_names.size() != schema->attributes.size())
		ILE("The rename operation caused the schema to have duplicate names");
	
	for(auto & tuple : relation->tuples)
		tuple->schema = schema;

	return relation.unbox();

}

/**
 * \Brief Finds the maximum value for the given column
 * \Note Ordering is not defined for text
 */
int64_t rm_maxRel(rm_object * lhs, const char * name) {

	Relation * rel = static_cast<Relation *>(lhs);

	size_t index = getColumnIndex(rel, name);

	if(rel->tuples.size() == 0)
		return RM_NULLINT;

	AnyValue max;
	max.type = rel->schema->attributes[index].type;
	switch(max.type){
	case TInt:
		max.intValue = std::numeric_limits<int64_t>::min();
		break;
	case TBool:
		max.boolValue = std::numeric_limits<int8_t>::min();
		break;
	// TODO should we handle text? If so, how do we report back 
	// that what we have found is a pointer to text, not an integer?
	default:
		ILE("Unknown type of column", name);
	}
	
	for(auto tup : rel->tuples)
		if(max < tup->values[index])
			max = tup->values[index];

	switch(max.type){
	case TInt:
		return max.intValue;
	case TBool:
		return max.boolValue;
	default:
		ILE("Unknown type of column", name);
	}	

}

/**
 * \Brief Finds the minimum value for the given column
 */
int64_t rm_minRel(rm_object * lhs, const char * name) {

	Relation * rel = static_cast<Relation *>(lhs);

	size_t index = getColumnIndex(rel, name);

	if(rel->tuples.size() == 0)
		return RM_NULLINT;

	AnyValue min;
	min.type = rel->schema->attributes[index].type;
	switch(min.type){
	case TInt:
		min.intValue = std::numeric_limits<int64_t>::max();
		break;
	case TBool:
		min.boolValue = std::numeric_limits<int8_t>::max();
		break;
	// TODO should we handle text? If so, how do we report back 
	// that what we have found is a pointer to text, not an integer?
	default:
		ILE("Unknown type of column", name);
	}
	
	for(auto tup : rel->tuples)
		if(tup->values[index] < min)
			min = tup->values[index];

	switch(min.type){
	case TInt:
		return min.intValue;
	case TBool:
		return min.boolValue;
	default:
		ILE("Unknown type of column", name);
	}
}

/**
 * \Brief Returns the sum of all values for the given column
 */
int64_t rm_addRel(rm_object * lhs, const char * name) {

	Relation * rel = static_cast<Relation *>(lhs);
	size_t index = getColumnIndex(rel, name);

	if(rel->schema->attributes[index].type != TInt)
		ILE("Addition is not supported for the type of column", name);

	int64_t ret = 0;
	for(auto tup : rel->tuples)
		ret += tup->values[index].intValue;
	
	return ret;
}

/**
 * \Brief Returns the product of all values for the given column
 */
int64_t rm_multRel(rm_object * lhs, const char * name) {
	Relation * rel = static_cast<Relation *>(lhs);
	size_t index = getColumnIndex(rel, name);

	if(rel->schema->attributes[index].type != TInt)
		ILE("Multiplication is not supported for the type of column", name);

	int64_t ret = 1;
	for(auto tup : rel->tuples)
		ret *= tup->values[index].intValue;
	
	return ret;
}

/**
 * \Brief Return the number of non-null entries in the given column
 */
int64_t rm_countRel(rm_object * lhs, const char * name) {

	Relation * rel = static_cast<Relation *>(lhs);

	size_t index = getColumnIndex(rel, name);
	int64_t count = 0;

	switch(rel->schema->attributes[index].type){
	case TInt:
		for(auto tup : rel->tuples)
			if(tup->values[index].intValue != RM_NULLINT)
				count++;
		break;
	case TBool:
		for(auto tup : rel->tuples)
			if(tup->values[index].boolValue != RM_NULLBOOL)
				count++;
		break;
	case TText:
		for(auto tup : rel->tuples)
			if(tup->values[index].objectValue.getAs<TextBase>() != &undef_text)
				count++;
		break;
	default:
		ILE("Unknown type of column", name);
	}

	return count;
}


/**
 * \Brief Creates a tuple from the given entries
 */
rm_object * rm_createTup(uint32_t count, TupEntry * entries) {
	RefPtr<Tuple> t = makeRef<Tuple>();
	
	RefPtr<Schema> schema = makeRef<Schema>();
	t->schema = schema;
	
	for(uint32_t i = 0; i < count; i++){
		// add entry to the tuple's schema 
		TupEntry te = entries[i];
		Attribute attribute {
			(Type) te.type, te.name
		};
		schema->attributes.push_back(attribute);
		
		// add entry to the tuple's values
		switch(te.type){
		case TInt:
			t->values.emplace_back(te.value);
			break;
		case TBool:
			t->values.emplace_back(static_cast<int8_t>(te.value));
			break;
		case TText: 
		{
			TextBase * text = reinterpret_cast<TextBase *>(te.value);
			RefPtr<rm_object> ref(text);
			t->values.emplace_back((Type) te.type, std::move(ref));
			break;
		}
		default:
			ILE("bad object type while creating tuple", te.type);
		}
	}

	// check if we have produced a schema with duplicate names
	std::set<std::string> schema_names;
	for(auto attribute : schema->attributes)
		schema_names.insert(attribute.name);
	if(schema_names.size() != schema->attributes.size())
		ILE("A tuple's schema cannot contain duplicate names");

	return t.unbox();
}

/**
 * \Brief Creates the relation containing a single tuple
 */
rm_object * rm_createRel(rm_object * tup) {

	Tuple * tuple = static_cast<Tuple *>(tup);

	if(!tuple) ILE("Null tuple in rm_createRel");
	if(!tuple->schema) ILE("Tuple has null schema in rm_createRel");

	RefPtr<Relation> rel = makeRef<Relation>();

	rel->schema = tuple->schema;
	rel->tuples.push_back(RefPtr<Tuple>(tuple));

	return rel.unbox();
}

/**
 * \Brief Fetch the value given by name from tup
 * \Note the value is saved in ret
 */
void rm_tupEntry(rm_object * tup, const char * name, AnyRet * ret) {
	Tuple * tuple = static_cast<Tuple *>(tup);
	Schema * schema = tuple->schema.get();

	size_t i;
	for(i = 0; i < schema->attributes.size(); i++)
		if(schema->attributes[i].name == name)
			break;

	if(i == schema->attributes.size()) ILE("Name", name, "was not found in the tuple's schema");
	
	AnyValue val = tuple->values[i];
	ret->type = val.type;
	switch(val.type){
	case TInt:
		ret->value = val.intValue;
		break;
	case TBool:
		ret->value = val.boolValue;
		break;
	case TText:
		val.objectValue->incref();
		ret->value = reinterpret_cast<uint64_t>(val.objectValue.get());
		break;
	default:
		ILE("Unhandled type in rm_tupEntry");
	}
	
}

/**
 * \Brief returns the tuple which is the union of lhs and rhs
 * \Note Shared values are taken from rhs, not lhs
 */
rm_object * rm_extendTup(rm_object * lhs_, rm_object * rhs_) {

	// note, we intentionally swap lhs_ and rhs_ 
	// to ensure that shared values are taken 
	// from rhs, not lhs
	Tuple * lhs = static_cast<Tuple *>(rhs_);
	Tuple * rhs = static_cast<Tuple *>(lhs_);

	RefPtr<Tuple> ret = makeRef<Tuple>();
	RefPtr<Schema> schema = makeRef<Schema>();
	ret->schema = schema;
	
	std::vector<std::string> there;
	ret->values = lhs->values;
	schema->attributes = lhs->schema->attributes;
	for (auto & a: lhs->schema->attributes) 
		there.push_back(a.name);
	std::sort(there.begin(), there.end());
	
	for (size_t i=0; i < rhs->values.size(); ++i) {
		if (std::binary_search(
				there.begin(), there.end(),
				rhs->schema->attributes[i].name)) continue;
		schema->attributes.push_back(rhs->schema->attributes[i]);
		ret->values.push_back(rhs->values[i]);
	}

	return ret.unbox();
}

/**
 * \Brief Creates a tuple whose schema does not contain name
 * If the original tuple's schema does not contain name, we error out
 */
rm_object * rm_tupRemove(rm_object * tup, const char * name) {

	Tuple * old_tup = static_cast<Tuple *>(tup);
	Schema * old_schema = old_tup->schema.get();
	
	RefPtr<Tuple> new_tup = makeRef<Tuple>();
	RefPtr<Schema> new_schema = makeRef<Schema>();
	new_tup->schema = new_schema;

	bool found_name = false;
	for(size_t i = 0; i < old_schema->attributes.size(); i++){
		if(old_schema->attributes[i].name == name){
			found_name = true;
			continue;
		}
		new_schema->attributes.push_back(old_schema->attributes[i]);
		new_tup->values.push_back(old_tup->values[i]);
	}

	if(!found_name) ILE("Name", name, "was not found in the tuple's schema");

	return new_tup.unbox();
}

/**
 * \Brief Checks whether or not the given tuple's schema contains the given name
 */
uint8_t rm_tupHasEntry(rm_object * tup, const char * name) {
	Tuple * tuple = static_cast<Tuple *>(tup);

	for(auto attribute : tuple->schema->attributes){
		if(name == attribute.name) return RM_TRUE;
	}

	return RM_FALSE;
}

/**
 * \Brief Checks whether or not the given relation's schema contains the given name
 */
uint8_t rm_relHasEntry(rm_object * rel, const char * name) {
	Relation * relation = static_cast<Relation *>(rel);
	if(!relation) ILE("Null relation passed to rm_relHasEntry");
	if(!relation->schema) ILE("Null schema for relation passed to rm_relHasEntry");

	for(auto attribute : relation->schema->attributes){
		if(name == attribute.name) return RM_TRUE; 
	}

	return RM_FALSE;
}

/**
 * \Brief Checks if lhs and ehs are identical
 * They must be completely identical; same schema, same number of tuples etc.
 */
uint8_t rm_equalRel(rm_object * lhs, rm_object * rhs) {
	Relation * l = static_cast<Relation *>(lhs);
	Relation * r = static_cast<Relation *>(rhs);

	if(l->schema->attributes.size() != r->schema->attributes.size()) return RM_FALSE;
	if(l->tuples.size() != r->tuples.size()) return RM_FALSE;

	// ensure schema equality
	std::vector<std::pair<std::string, size_t>> l_indices;
	std::vector<std::pair<std::string, size_t>> r_indices;
	
	for(size_t i = 0; i < l->schema->attributes.size(); i++){
		l_indices.emplace_back(l->schema->attributes[i].name, i);
		r_indices.emplace_back(r->schema->attributes[i].name, i);
	}

	std::sort(l_indices.begin(), l_indices.end());
	std::sort(r_indices.begin(), r_indices.end());

	for(size_t i = 0; i < l->schema->attributes.size(); i++){
		size_t left_index = l_indices[i].second;
		size_t right_index = r_indices[i].second;
		if(l->schema->attributes[left_index].name != 
		   r->schema->attributes[right_index].name || 
		   l->schema->attributes[left_index].type !=
		   r->schema->attributes[right_index].type)
			return RM_FALSE;
	}

	// sort the rows in each schema before checking equality
	std::sort(l->tuples.begin(), l->tuples.end(),
			  [&](const RefPtr<Tuple> & _a,
				  const RefPtr<Tuple> & _b)->bool{
				  Tuple * a = _a.get(), * b = _b.get();
				  if(a->values.size() != b->values.size())
					  return a->values.size() < b->values.size();
				  for(size_t i = 0; i < a->values.size(); i++){
					  size_t left_index = l_indices[i].second;
					  if (!(a->values[left_index] == b->values[left_index]))
						  return a->values[left_index] < b->values[left_index];
				  }
				  return false;
			  });
	std::sort(r->tuples.begin(), r->tuples.end(),
			  [&](const RefPtr<Tuple> & _a,
				  const RefPtr<Tuple> & _b)->bool{
				  Tuple * a = _a.get(), * b = _b.get();
				  if(a->values.size() != b->values.size())
					  return a->values.size() < b->values.size();
				  for(size_t i = 0; i < a->values.size(); i++){
					  size_t right_index = r_indices[i].second;
					  if (!(a->values[right_index] == b->values[right_index]))
						  return a->values[right_index] < b->values[right_index];
				  }
				  return false;
			  });

	// ensure tuple equality
	for(size_t i = 0; i < l->tuples.size(); i++){	
		for(size_t j = 0; j < l->schema->attributes.size(); j++){
			size_t left_index = l_indices[j].second;
			size_t right_index = r_indices[j].second;
			if(! (l->tuples[i]->values[left_index] == r->tuples[i]->values[right_index]))
				return RM_FALSE;
		}
	}
	
	return RM_TRUE;
}

/**
 * \Brief Checks if lhs and rhs are identical
 * First we check if their schema lengths are identical
 * Then we sort their schemas for easy comparison
 * Finally we check if both schema names and tuple values are equal
 */
uint8_t rm_equalTup(rm_object * lhs, rm_object * rhs) {

	// check sizes of schemas
	Tuple * lt = static_cast<Tuple *>(lhs); // left tuple
	Tuple * rt = static_cast<Tuple *>(rhs); // right tuple

	Schema * lts = lt->schema.get();
	Schema * rts = rt->schema.get();

	if(lts->attributes.size() != rts->attributes.size())
		return RM_FALSE;

	// create a vector of pairs (schemaname, index) for each tuple
	// these vectors will be sorted so that we can easily
	// compare the tuples
	std::vector<std::pair<std::string, size_t>> lt_indices;
	std::vector<std::pair<std::string, size_t>> rt_indices;
	
	for(size_t i = 0; i < lts->attributes.size(); i++){
		lt_indices.emplace_back(lts->attributes[i].name, i);
		rt_indices.emplace_back(rts->attributes[i].name, i);
	}

	std::sort(lt_indices.begin(), lt_indices.end());
	std::sort(rt_indices.begin(), rt_indices.end());
	
	for(size_t i = 0; i < lts->attributes.size(); i++){
		size_t left_index = lt_indices[i].second;
		size_t right_index = rt_indices[i].second;

		// different schemas?
		if(lts->attributes[left_index].name != 
		   rts->attributes[right_index].name ||
		   lts->attributes[left_index].type !=
		   rts->attributes[right_index].type)
			return RM_FALSE;

		// different values?
		AnyValue avl = lt->values[left_index];
		AnyValue avr = rt->values[right_index];
		if (!(avl == avr)) return RM_FALSE;
	}

	return RM_TRUE;
}

/**
 * \Brief runs 'func' on each tuple in 'rel_' and returns a relation which is the union of the results
 */
rm_object * rm_forAll(rm_object * rel_, rm_object * func){

	Relation * rel = static_cast<Relation *>(rel_);
	RefPtr<Relation> ret = makeRef<Relation>();
	ret->schema = makeRef<Schema>();

	RefPtr<Relation> arg_rel = makeRef<Relation>();
	arg_rel->schema = makeRef<Schema>();
	
	bool ret_initialized = false;
	
	for(size_t i = 0; i < rel->tuples.size(); i++){

		Tuple * tup = rel->tuples[i].get();
		int64_t arg_tup = reinterpret_cast<int64_t>(tup);
		int64_t arg_rel_as_int = reinterpret_cast<int64_t>(arg_rel.get());

		AnyRet retval;
		FuncBase * base = (FuncBase *) func;
		typedef void * (*t0)(FuncBase *, AnyRet *, int64_t, int8_t, int64_t, int8_t);

		((t0) base->func)(base, &retval, arg_tup, TTup, arg_rel_as_int, TRel);

		if(retval.type != TRel) ILE("Should not happen");
		Relation * _result = reinterpret_cast<Relation *>(retval.value);
		RefPtr<Relation> result = RefPtr<Relation>::steal(_result);

		if(!ret_initialized){
			ret->schema->attributes = result->schema->attributes;
			ret_initialized = true;
		}else if(!schemaEquals(ret->schema.get(), result->schema.get()))
			ILE("The relations returned must all have identical schemas");

		for(auto & tuple : result->tuples)
			ret->tuples.push_back(tuple);
	}

	removeDuplicateRows(ret.get());
	return ret.unbox();
}

/**
 * \Brief Factors 'relations' on 'col_names' using 'func'
 * First we do some sanity checks on the input
 * Next, we confirm that each name in 'col_names' exists in each given relation, and that all such columns have equal types
 * Then we sort all relations on their tuples restricted to 'col_names'
 * We keep a pointer into each relation. At first it points to the first row.
 * We find the smallest restricted tuple currently pointed to by our relation-pointers
 * Then we find the interval in every relation which has restricted tuples equal to the minimal one.
 * For each relation, using this interval, we create a relation which will be an argument to 'func'
 * Once we have calculated all arguments from all relations, we call 'func' and union the resulting relation into our return-relation
 * Finally, we remove duplicates and return the return-relation
 */
rm_object * rm_factorRel(uint32_t num_col_names, char ** col_names, uint32_t num_relations, rm_object ** relations, rm_object * func){

	// preliminary checks
	
	// when factor is given no columns, it should implicitly use all of them;
	// this is equivalent to a forall
	if(num_col_names == 0){
		if(num_relations != 1)
			ILE("The forall operator only supports exactly one relation");

		return rm_forAll(relations[0], func);
	}

	if(num_relations < 1) ILE("No relations were given to factor");
	
	if(num_relations > 16) 
		ILE("It is not possible to factor more than 16 relations at once"); // TODO increase number with the code generator

	std::vector<std::string> names;
	for(uint32_t i = 0; i < num_col_names; i++)
		names.push_back(col_names[i]);

	std::sort(names.begin(), names.end());
	if(std::adjacent_find(names.begin(), names.end()) != names.end())
		ILE("Duplicate column names provided to factor");
		
	// confirm that all the column names exist in all
	// the relations
	
	// rel_indices is a vector containing a vector for each relation;
	// the inner vectors keep the indices of each column where
	// a name from 'names' can be found
	std::vector<std::vector<size_t>> rel_indices;

	// this vector contains indices of columns that are not in 'names', for each relation.
	std::vector<std::vector<size_t>> rel_nonname_indices;
	
	for(size_t i = 0; i < num_relations; i++){
		
		Relation * cur_rel = static_cast<Relation *>(relations[i]);
		rel_indices.push_back(std::vector<size_t>());
		rel_nonname_indices.push_back(std::vector<size_t>());
		
		std::vector< std::pair<std::string, size_t> > col_indices;
		for(size_t j = 0; j < cur_rel->schema->attributes.size(); j++)
			col_indices.emplace_back(cur_rel->schema->attributes[j].name, j);
		std::sort(col_indices.begin(), col_indices.end());

		// both names and col_indices are sorted at this point
		for(size_t j = 0, k = 0; k < col_indices.size(); k++){
			size_t name_index = col_indices[k].second;
			if(j >= names.size()){
				rel_nonname_indices[i].push_back(name_index);
				continue;
			}
			std::string wanted_name = names[j];
			std::string current_name = col_indices[k].first;
			if(wanted_name == current_name){
				rel_indices[i].push_back(name_index);
				j++;
			} else 
				rel_nonname_indices[i].push_back(name_index);

		}

		if(rel_indices[i].size() != names.size())
			ILE("All of the relations' schemas must contain all the given columns");
	}
	
	// ensure that the type of columns in each schema is equal to the others

	if(num_relations == 0) ILE("This should not be reachable");
	Relation * rel1 = static_cast<Relation *>(relations[0]);
	for(size_t i = 1; i < num_relations; i++){
		Relation * rel2 = static_cast<Relation *>(relations[i]);
		for(size_t j = 0; j < names.size(); j++){
			size_t index1 = rel_indices[0][j];
			size_t index2 = rel_indices[i][j];
			if(rel1->schema->attributes[index1].type !=
			   rel2->schema->attributes[index2].type)
				ILE("All of the given columns must have equal types in all given relations");
		}
	}
	
	// sort all the relations on their restricted tuples

	for(size_t i = 0; i < num_relations; i++){
		Relation * cur_rel = static_cast<Relation *>(relations[i]);

		std::sort(cur_rel->tuples.begin(), cur_rel->tuples.end(),
				  [&](const RefPtr<Tuple> & a,
					  const RefPtr<Tuple> & b)->bool{
					  return restrict(a, rel_indices[i]) <
						  restrict(b, rel_indices[i]);
				  });
	}

	// precompute the schemas of the smaller relations,
	// which will later be passed to func

	std::vector< RefPtr<Schema> > arg_schemas;
	for(size_t i = 0; i < num_relations; i++){
		Relation * cur_rel = static_cast<Relation *>(relations[i]);
		RefPtr<Schema> arg_schema = makeRef<Schema>();
		for(size_t j = 0; j < rel_nonname_indices[i].size(); j++){
			size_t attribute_index = rel_nonname_indices[i][j];
			arg_schema->attributes.push_back(cur_rel->schema->attributes[attribute_index]);
		}
		arg_schemas.push_back(arg_schema);
	}

	// precompute the minimum restricted tuple's schema
	// by setting it to the schema of the first relation, plus-projected
	// onto the wanted column names

	RefPtr<Schema> min_restriction_schema = makeRef<Schema>();
	Relation * first_rel = static_cast<Relation *>(relations[0]);
	for(size_t i = 0; i < rel_indices[0].size(); i++){
		size_t index = rel_indices[0][i];
		min_restriction_schema->attributes.push_back(first_rel->schema->attributes[index]);
	}

	// while there are rows left, compute tuple and smaller relations
	// and call func with them as arguments. Append the result
	// of evaluating func to the return relation.

	// this will be returned in the end
	bool ret_initialized = false;
	RefPtr<Relation> ret = makeRef<Relation>();
	ret->schema = makeRef<Schema>();

	// maintain start- and end-pointers to intervals in each relation
	std::vector<size_t> row_indices(num_relations, 0);
	
	while(1){

		// loop condition: are there rows left to process?
		bool proceed = false;
		for(size_t i = 0; i < num_relations; i++){
			Relation * rel = static_cast<Relation *>(relations[i]);
			if(row_indices[i] < rel->tuples.size()){
				proceed = true;
				break;
			}
		}
		if(!proceed) break;

		// find the minimal restriction currently pointed to
		RefPtr<Tuple> min_restriction = makeRef<Tuple>();
		bool initialized = false;
		for(size_t i = 0; i < num_relations; i++){
			Relation * rel = static_cast<Relation *>(relations[i]);
			size_t cur_tup_index = row_indices[i];
			if(cur_tup_index == rel->tuples.size()) continue;
			RefPtr<Tuple> cur_tup = rel->tuples[cur_tup_index];
			Tuple restriction = restrict(cur_tup, rel_indices[i]);
			if(!initialized || restriction < *min_restriction){
				min_restriction->values = restriction.values;
				initialized = true;
			}
		}
		if(!initialized) ILE("Should not happen");		
		min_restriction->schema = min_restriction_schema;

		// for each relation, compute the smaller relation
		// which will be an argument to func later
		
		std::vector< RefPtr<Relation> > args;
		for(size_t i = 0; i < num_relations; i++){

			Relation * cur_rel = static_cast<Relation *>(relations[i]);
			RefPtr<Relation> arg_rel = makeRef<Relation>();
			arg_rel->schema = arg_schemas[i];

			size_t row_index;
			for(row_index = row_indices[i]; row_index < cur_rel->tuples.size()
					&& restrict(cur_rel->tuples[row_index], rel_indices[i]) == *min_restriction;
				row_index++){
				RefPtr<Tuple> tup = makeRef<Tuple>();
				tup->schema = arg_schemas[i];
				tup->values = restrict(cur_rel->tuples[row_index], rel_nonname_indices[i]).values;
				arg_rel->tuples.push_back(std::move(tup));
			}

			args.push_back(std::move(arg_rel));
			row_indices[i] = row_index;
		}

		// now we have the arguments; call func

		if(args.size() != num_relations)
			ILE("Should not happen");

		AnyRet retval;
		FuncBase * base = (FuncBase *) func;
		std::vector<int64_t> args_as_ints;

		for(size_t i = 0; i < args.size(); i++)
			args_as_ints.push_back(reinterpret_cast<int64_t>(args[i].get()));

		if(args_as_ints.size() != num_relations)
			ILE("Should not happen");

		int64_t tuple = reinterpret_cast<int64_t>(min_restriction.get());

		run(base, &retval, base->func, tuple, args_as_ints);
		if(retval.type != TRel) ILE("Should not happen");

		Relation * _result = reinterpret_cast<Relation *>(retval.value);
		RefPtr<Relation> result = RefPtr<Relation>::steal(_result);

		if(!ret_initialized){
			ret->schema->attributes = result->schema->attributes;
			ret_initialized = true;
		}else if(!schemaEquals(ret->schema.get(), result->schema.get()))
			ILE("The relations returned must all have identical schemas");

		for(auto & tuple : result->tuples)
			ret->tuples.push_back(tuple);
	}

	// TODO Q: if there are no rows to process, we return the zero relation. Is this correct?

	removeDuplicateRows(ret.get());	
	return ret.unbox();
}



} // extern "C"
