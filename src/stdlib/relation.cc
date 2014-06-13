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
				tuple.getAs<Tuple>()->values.emplace_back(line != "false");
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


rm_object * rm_joinRel(rm_object * lhs, rm_object * rhs) {
	//TODO
	lhs->ref_cnt++;
	return lhs;
}

rm_object * rm_unionRel(rm_object * lhs, rm_object * rhs) {
	//TODO
	lhs->ref_cnt++;
	return lhs;
}

rm_object * rm_diffRel(rm_object * lhs, rm_object * rhs) {
	//TODO
	lhs->ref_cnt++;
	return lhs;
}

rm_object * rm_selectRel(rm_object * rel, rm_object * func) {
	//TODO
	rel->ref_cnt++;
	return rel;
}

rm_object * rm_projectPlusRel(rm_object * rel, uint32_t name_count, const char ** names) {
	//TODO
	rel->ref_cnt++;
	return rel;
}

rm_object * rm_projectMinusRel(rm_object * rel, uint32_t name_count, const char ** names) {
	//TODO
	rel->ref_cnt++;
	return rel;
}

rm_object * rm_renameRel(rm_object * rel, uint32_t name_count, const char ** names) {
	//TODO
	rel->ref_cnt++;
	return rel;
}

int64_t rm_maxRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}

int64_t rm_minRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}

int64_t rm_addRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}

int64_t rm_multRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}

int64_t rm_countRel(rm_object * lhs, const char * name) {
	//TODO
	return std::numeric_limits<int64_t>::min();
}



rm_object * rm_createTup(uint32_t count, TupEntry * entries) {
	//TODO IMPLEMENT ME
	Tuple * t=new Tuple();
	t->ref_cnt = 1;
	registerAllocation(t);
	return t;
}

rm_object * rm_createRel(rm_object * tup) {
	//TODO
	Relation * r = new Relation();
	r->ref_cnt = 1;
	registerAllocation(r);
	return r;
}

void rm_tupEntry(rm_object * tup, const char * name, AnyRet * ret) {
	//TODO fix me
	ret->value=0;
	ret->type=TInt;
}

rm_object * rm_extendTup(rm_object * lhs, rm_object * rhs) {
	//TODO fixme
	lhs->ref_cnt++;
	return lhs;
}

rm_object * rm_tupRemove(rm_object * tup, const char * name) {
	//TODO fixme
	tup->ref_cnt++;
	return tup;
}

uint8_t rm_tupHasEntry(rm_object * tup, const char * name) {
	//TODO fixme
	return 0;
}

uint8_t rm_relHasEntry(rm_object * tup, const char * name) {
	//TODO fixme
	return 0;
}

uint8_t rm_equalRel(rm_object * lhs, rm_object * rhs) {
	return 3; //TODO 
}


uint8_t rm_equalTup(rm_object * lhs, rm_object * rhs) {
	return 3; //TODO 
}

} // extern "C"
