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

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <stdlib/refptr.hh>
#include <stdlib/text.hh>
#include <stdlib/relation.hh>
#include <stdlib/callback.hh>
#include <stdlib/anyvalue.hh>
#include <stdlib/ile.hh>

namespace {
using namespace rasmus::stdlib;

int rm_itemWidth(AnyValue av){
	// TODO implement this method properly
	switch(av.type){
	case TBool:
		return 5; // strlen("false")
	case TInt:
		return std::to_string(av.intValue).size();
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
	// calculate widths
	Schema * schema = relation->schema.getAs<Schema>();
	size_t num_attributes = schema->attributes.size();
	std::vector<int> widths(num_attributes, 0);

	// widths for tuples
	for(auto & tuple : relation->tuples){
		int i = 0;		
		for(auto & item : tuple.getAs<Tuple>()->values){
			widths[i] = std::max(widths[i], rm_itemWidth(item));
			i++;
		}
	}

	// width for schema names
	int i = 0;
	for(auto & attribute : relation->schema.getAs<Schema>()->attributes){
		widths[i] = std::max<int>(widths[i], attribute.name.size());
		i++;
	}
	
	size_t total_width = std::accumulate(begin(widths), end(widths), 0);
	total_width += 1 + widths.size() * 3; 
    // 1 for beginning |, 3 per field for 2 spaces and a |

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
	for(auto & tuple : relation->tuples){
		i = 0;
		out << '|';
		for(auto & value : tuple.getAs<Tuple>()->values){
			switch(value.type){
			case TInt:
				out << ' ' << std::right << std::setw(widths[i]) << value.intValue;
				break;
			case TBool:
				out << ' ' << std::left << std::setw(widths[i]) << (value.boolValue ? "true" : "false");
				break;
			case TText:
				out << ' ' << std::left << std::setw(widths[i]) << textToString(value.objectValue.getAs<TextBase>()); 
				break;
			default:
				ILE("Unhandled type", value.type);
			}
			out << " |";
			i++;
		}
		out << std::endl;
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
	outFile << relation->schema.getAs<Schema>()->attributes.size() << std::endl;
	for(auto & attribute : relation->schema.getAs<Schema>()->attributes){
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
		for(auto & value : tuple.getAs<Tuple>()->values){
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
	RefPtr<rm_object> schema;

	if(!(inFile >> num_columns)){
		std::cerr << "could not read number of attributes from file "  << std::endl;
		exit(EXIT_FAILURE);
	}

	schema = RefPtr<rm_object>(new Schema());
	
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

		schema.getAs<Schema>()->attributes.push_back(std::move(attribute));

	}

	{
		std::string _;
		getline(inFile, _);
	}

	RefPtr<rm_object> relations(new Relation());
	relations.getAs<Relation>()->schema = schema;

	bool done = false;

	while(!done){
		
		RefPtr<rm_object> tuple(new Tuple());
		tuple.getAs<Tuple>()->schema = schema;

		for(auto & attribute : schema.getAs<Schema>()->attributes){
			
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
				tuple.getAs<Tuple>()->values.emplace_back(value);
				break;
			}
			case TBool:
				tuple.getAs<Tuple>()->values.emplace_back((line != "false") ? RM_TRUE : RM_FALSE);
				break;
			case TText:
				tuple.getAs<Tuple>()->values.emplace_back(TText, RefPtr<rm_object>::steal(rm_getConstText(line.c_str())));
				break;
			default:
				std::cerr << "internal library error" << std::endl;
				exit(EXIT_FAILURE);			
				break;
			}
			
		}

		if(done) break;
		
		relations.getAs<Relation>()->tuples.push_back(std::move(tuple));

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


} //namespace stdlib
} //namespace rasmus

extern "C" {

//TODO fix me
Relation zero_rel;
//TODO fix me
Relation one_rel;


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
 * First we identify columns which are shared by lhs and rhs (e.g. by sorting)
 * Then we sort lhs and rhs by the shared columns
 * Then we scan through lhs and rhs and output the union of their shared columns
 * \Note if no columns are shared, the full Carthesian product is returned
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
 * \Note we need to remove duplicate rows
 */
rm_object * rm_unionRel(rm_object * lhs, rm_object * rhs) {
	//TODO
	lhs->ref_cnt++;
	return lhs;
}

/**
 * \Brief Return the difference of lhs and rhs
 * Again assert that lhs and rhs have the same schema
 * Then return the set difference of the rows in lhs and rhs (lhs - rhs)
 */
rm_object * rm_diffRel(rm_object * lhs, rm_object * rhs) {
	//TODO
	lhs->ref_cnt++;
	return lhs;
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
rm_object * rm_projectPlusRel(rm_object * rel, uint32_t name_count, const char ** names) {
	//TODO
	rel->ref_cnt++;
	return rel;
}

/** \Brief Projects rel into all names except the given set of names
 */
rm_object * rm_projectMinusRel(rm_object * rel, uint32_t name_count, const char ** names) {
	//TODO
	// Can be implemented as a call to rm_projectPlusRel
	rel->ref_cnt++;
	return rel;
}

/**
 * \Brief Replace the names in the relation with a set of replacement names
 * \Note "names" has twice the length of name_count; it holds both the old names and the new ones
 */
rm_object * rm_renameRel(rm_object * rel, uint32_t name_count, const char ** names) {

	Relation * old_rel = static_cast<Relation *>(rel);

	Relation * relation = new Relation();
	relation->ref_cnt = 1;
	registerAllocation(relation);
		
	for(auto & old_tup : old_rel->tuples){
		Tuple * new_tup = new Tuple();
		registerAllocation(new_tup);
		new_tup->values = old_tup.getAs<Tuple>()->values;
		// tuple's schema will be updated later
		relation->tuples.push_back(RefPtr<rm_object>(new_tup));
	}

	Schema * schema;

	// no-one else is using the relation's schema
	if(old_rel->schema->ref_cnt == 1){
		schema = old_rel->schema.getAs<Schema>();
		relation->schema = old_rel->schema;

	// construct a new schema, copying the old one
	}else{

		schema = new Schema;
		registerAllocation(schema);
		relation->schema = RefPtr<rm_object>(schema);

		schema->attributes = old_rel->schema.getAs<Schema>()->attributes;
		
	}

	if(schema->attributes.size() < name_count)
		ILE("Wrong number of names given to rm_renameRel - name_count:", name_count, "schema size:", schema->attributes.size());
	
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
	
	// replace each tuple's schema
	// TODO Q can we assume that the schema is shared between tuples and relation? If so, this can be skipped
	for(auto & tuple : relation->tuples){
		tuple.getAs<Tuple>()->schema = relation->schema;
	}

	return relation;

}

/**
 * \Brief Finds the maximum value for the given column
 * \Note Ordering is not defined for text
 */
int64_t rm_maxRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}

/**
 * \Brief Finds the minimum value for the given column
 */
int64_t rm_minRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}

/**
 * \Brief Returns the sum of all values for the given column
 */
int64_t rm_addRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}

/**
 * \Brief Returns the product of all values for the given column
 */
int64_t rm_multRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}

/**
 * \Brief Return the number of unique entries in the given column
 */
int64_t rm_countRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}


/**
 * \Brief Creates a tuple from the given entries
 */
rm_object * rm_createTup(uint32_t count, TupEntry * entries) {
	Tuple * t = new Tuple();
	t->ref_cnt = 1;
	registerAllocation(t);
	
	Schema * schema = new Schema;
	registerAllocation(schema);
	t->schema = RefPtr<rm_object>(schema);
	
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

	return t;
}

/**
 * \Brief Creates the relation containing a single tuple
 */
rm_object * rm_createRel(rm_object * tup) {

	Tuple * tuple = static_cast<Tuple *>(tup);

	if(!tuple) ILE("Null tuple in rm_createRel");
	if(!tuple->schema) ILE("Tuple has null schema in rm_createRel");

	Relation * rel = new Relation();
	rel->ref_cnt = 1;
	registerAllocation(rel);

	rel->schema = tuple->schema;
	rel->tuples.push_back(RefPtr<rm_object>(tup));

	return rel;
}

/**
 * \Brief Fetch the value given by name from tup
 * \Note the value is saved in ret
 */
void rm_tupEntry(rm_object * tup, const char * name, AnyRet * ret) {
	Tuple * tuple = static_cast<Tuple *>(tup);
	Schema * schema = tuple->schema.getAs<Schema>();

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
		ret->value = reinterpret_cast<uint64_t>(&val.objectValue);
		break;
	default:
		ILE("Unhandled type in rm_tupEntry");
	}
	
}

/**
 * \Brief returns the tuple which is the union of lhs and rhs
 * \Note Shared values are taken from lhs, not rhs
 */
rm_object * rm_extendTup(rm_object * lhs, rm_object * rhs) {
	//TODO fixme
	lhs->ref_cnt++;
	return lhs;
}

/**
 * \Brief Creates a tuple whose schema does not contain name
 */
rm_object * rm_tupRemove(rm_object * tup, const char * name) {
	//TODO fixme
	tup->ref_cnt++;
	return tup;
}

/**
 * \Brief Checks whether or not the given tuple's schema contains the given name
 */
uint8_t rm_tupHasEntry(rm_object * tup, const char * name) {
	Tuple * tuple = static_cast<Tuple *>(tup);

	for(auto attribute : tuple->schema.getAs<Schema>()->attributes){
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

	for(auto attribute : relation->schema.getAs<Schema>()->attributes){
		if(name == attribute.name) return RM_TRUE; 
	}

	return RM_FALSE;
}

/**
 * \Brief Checks if lhs and ehs are identical
 * They must be completely identical; same schema, same number of tuples etc.
 */
uint8_t rm_equalRel(rm_object * lhs, rm_object * rhs) {
	return 3; //TODO 
}

/**
 * \Brief Checks if lhs and rhs are identical
 */
uint8_t rm_equalTup(rm_object * lhs, rm_object * rhs) {
	return 3; //TODO 
}

} // extern "C"
