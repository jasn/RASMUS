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
					printTextToStream(value.objectValue.getAs<TextBase>(), out);
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
	//TODO
	lhs->ref_cnt++;
	return lhs;
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

	for(size_t i = 0; i < l->schema->attributes.size(); i++){
		size_t left_index = l_indices[i].second;
		size_t right_index = r_indices[i].second;
		if(l->schema->attributes[left_index].name != 
		   r->schema->attributes[right_index].name || 
		   l->schema->attributes[left_index].type !=
		   r->schema->attributes[right_index].type)
			ILE("The given schemas are not equal!");
	}
	
	// find the set difference between the tuples in lhs and rhs
	std::set<RefPtr<Tuple>> ls(l->tuples.begin(), l->tuples.end());
	std::set<RefPtr<Tuple>> rs(r->tuples.begin(), r->tuples.end());

	RefPtr<Relation> ret = makeRef<Relation>();
	ret->schema = l->schema;

	std::set_difference(ls.begin(), ls.end(), rs.begin(), rs.end(),
						std::inserter(ret->tuples, ret->tuples.end()),
						cmpTupPtrs);

	return ret.unbox();
}

/**
 * Evaluate func on every tuple in rel
 * If this evaluates to RM_TRUE, the tuple is included in the result
 */
rm_object * rm_selectRel(rm_object * rel, rm_object * func) {
	//TODO
	// Spørg Jakob hvordan funktion skal kaldes. 
	// dette laves sent
	rel->ref_cnt++;
	return rel;
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
		uint8_t found = 0;
		for(auto & attribute : schema->attributes){
			if(attribute.name == old_name){
				attribute.name = new_name;
				found = 1;
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
 * \Note Shared values are taken from lhs, not rhs
 */
rm_object * rm_extendTup(rm_object * lhs_, rm_object * rhs_) {
	Tuple * lhs = static_cast<Tuple *>(lhs_);
	Tuple * rhs = static_cast<Tuple *>(rhs_);

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

	bool found_name = 0;
	for(size_t i = 0; i < old_schema->attributes.size(); i++){
		if(old_schema->attributes[i].name == name){
			found_name = 1;
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

} // extern "C"
