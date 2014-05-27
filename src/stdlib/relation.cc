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
#include "inner.hh"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace {

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
	Schema(): rm_object(LType::schema){};
};

/* used as a container class for tuples; each tuple has a number
   of AnyValues which can either be a TInt, TText or TBool
 */
struct AnyValue {
	Type type;
	union {
		int64_t intValue;
		bool boolValue;
		RefPtr objectValue;
	};
	
	AnyValue(int64_t value): type(TInt), intValue(value) {}
	AnyValue(bool value): type(TBool), boolValue(value) {}
	AnyValue(Type type, RefPtr value): type(type), objectValue(value) {}

	AnyValue(const AnyValue & other): type(other.type) {
		switch(type){
		case TInt:
			intValue = other.intValue;
			break;
		case TBool:
			boolValue = other.boolValue;
			break;
		default:
			new (&objectValue) RefPtr(other.objectValue);
			break;
		}

	}
	
	AnyValue(AnyValue && other): type(other.type) {
		switch(type){
		case TInt:
			intValue = other.intValue;
			break;
		case TBool:
			boolValue = other.boolValue;
			break;
		default:
			new (&objectValue) RefPtr(std::move(other.objectValue));
			break;
		}
	}

	AnyValue & operator= (const AnyValue & other){
		this->~AnyValue();
		new (this) AnyValue(other);
		return *this;
	}
	
	AnyValue & operator= ( AnyValue && other){
		this->~AnyValue();
		new (this) AnyValue(std::move(other));
		return *this;
	}
	
	~AnyValue() {
		switch (type) {
		case TInt:
		case TBool:
			break;
		default:
			objectValue.~RefPtr();
		}
	}
};

/* a tuple consists of a schema and a set of values 
   with data corresponding to the schema.
 */
class Tuple: public rm_object {
public:
	RefPtr schema;
	std::vector<AnyValue> values;
	Tuple(): rm_object(LType::tuple){};
};

/* a relation has a schema and some amount of tuples; 
   the tuples contain data corresponding to the schema
 */
class Relation: public rm_object {
public:
	std::vector<RefPtr> tuples;
	RefPtr schema;
	Relation(): rm_object(LType::relation){};
};

} // nameless namespace

extern "C" {


int rm_itemWidth(AnyValue av){

	// TODO implement this method properly
	switch(av.type){
	case TBool:
		return 5; // strlen("false")
		break;
	case TInt:
		return std::to_string(av.intValue).size();
		break;
	default:
		return av.objectValue.getAs<TextBase>()->length;
		break;
	}

}

void rm_printRel(rm_object * ptr) {

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
	
	std::cout << std::string(total_width, '-') << std::endl;
	std::cout << "|";

	i = 0;
	for(auto & attribute : schema->attributes){
		std::cout << ' ' << std::left << std::setw(widths[i]) << attribute.name <<  " |";
		i++;
	}
	
	std::cout << std::endl;
		
	// print body

	std::cout << std::string(total_width, '-') << std::endl;

	for(auto & tuple : relation->tuples){
		
		i = 0;

		std::cout << '|';

		for(auto & value : tuple.getAs<Tuple>()->values){

			switch(value.type){
			case TInt:
				std::cout << ' ' << std::right << std::setw(widths[i]) << value.intValue;
				break;
			case TBool:
				std::cout << ' ' << std::left << std::setw(widths[i]) << (value.boolValue ? "true" : "false");
				break;
			case TText:
				std::cout << ' ' << std::left << std::setw(widths[i]) << rm_textToString(value.objectValue.get()); 
				break;
			}

			std::cout << " |";

			i++;

		}

		std::cout << std::endl;

	}
	
	std::cout << std::string(total_width, '-') << std::endl;

}

/*  outputs the given relation to the given stream. 

	The output format is as given in the RASMUS 
	user manual:

	First the size of the schema is output, then
	the type and name of each attribute, and finally
	the values of the tuples in the relation.
 */
void rm_saveRelToStream(rm_object * o, std::ostream & outFile){
	
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
			// TODO add error checking here
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
				rm_printTextToStream(value.objectValue.get(), outFile);
				outFile << std::endl;
				break;
			}
		}
	}
	
}	

/* wrapper function */
void rm_saveRel(rm_object * o, const char * name) {
	std::ofstream outFile(name);
	rm_saveRelToStream(o, outFile);
}


rm_object * rm_joinRel(rm_object * lhs, rm_object * rhs) {return nullptr;}
rm_object * rm_unionRel(rm_object * lhs, rm_object * rhs) {return nullptr;}


/*  parses a relation given by an input stream and returns 
	it. The format is as given in the RASMUS user manual. 

	The attribute types and names are saved in relations->schema
	and relations->tuples contains the tuples for the relation
*/
rm_object * rm_loadRelFromStream(std::istream & inFile){

	size_t num_columns;
	RefPtr schema;

	if(!(inFile >> num_columns)){
		std::cerr << "could not read number of attributes from file "  << std::endl;
		exit(EXIT_FAILURE);
	}

	schema = RefPtr(new Schema());
	
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

	RefPtr relations(new Relation());
	relations.getAs<Relation>()->schema = schema;

	bool done = false;

	while(!done){
		
		RefPtr tuple(new Tuple());
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
				tuple.getAs<Tuple>()->values.emplace_back(TText, RefPtr::steal(rm_getConstText(line.c_str())));
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

/* wrapper function */
rm_object * rm_loadRel(const char * name) {
	std::ifstream inFile(name);
	return rm_loadRelFromStream(inFile);
}


} // extern "C"
