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
#include <set>
#include <cmath>
#include <stdlib/refptr.hh>
#include <stdlib/text.hh>
#include <stdlib/relation.hh>
#include <stdlib/callback.hh>
#include <stdlib/anyvalue.hh>
#include <stdlib/ile.hh>
#include <stdlib/abort.hh>
#include <numeric>

namespace {
using namespace rasmus::stdlib;

size_t utf8strlen(std::string str){
	size_t len = 0;
	for(size_t i = 0; i < str.size(); ){
		len++;
		const char & c = str.at(i);
		if((c & 0x80) == 0) i++;
		else if((c & 0xE0) == 0xC0) i += 2;
		else if((c & 0xF0) == 0xE0) i += 3;
		else if((c & 0xF8) == 0xF0) i += 4;
		else if((c & 0xFC) == 0xF8) i += 5;
		else if((c & 0xFE) == 0xFC) i += 6;
		else ILE("Invalid utf8-char given to utf8strlen");
	}
	return len;
}

void escapeNewlines(std::string & text){
	size_t sp = 0;
	while((sp = text.find('\n')) != std::string::npos){
		text.replace(sp, 1, "\\n");
		sp += 2; // strlen("\\n")
	}
}

int rm_itemWidth(AnyValue av){
	std::string str;
	std::wstring widestr;
	switch(av.type){
	case TBool:
		if(av.boolValue == RM_NULLBOOL) return 6; // strlen("?-Bool")
		else return 5; // strlen("false")
	case TInt:
		if(av.intValue == RM_NULLINT) return 5; // strlen("?-Int")
		else return std::to_string(av.intValue).size();
	case TFloat:
		if(std::isnan(av.floatValue)) return 7; // strlen("?-Float")
		else return std::to_string(av.floatValue).size();
	case TText:
	{
		std::string str = textToString(av.objectValue.getAs<TextBase>());
		escapeNewlines(str);
		return utf8strlen(str);
	}
	default:
		ILE("Unhandled type");
	}
}

} // nameless namespace

namespace rasmus {
namespace stdlib {
/**
 * Makes a copy of the relation \param rel
 *
 */
RefPtr<Relation> copyRelation(RefPtr<Relation> rel) {
	RefPtr<Relation> ret = makeRef<Relation>();
	ret->schema = rel->schema;
	ret->permutation = rel->permutation;
	ret->tuples = rel->tuples;
	return ret;
}


void printRelationToStream(rm_object * ptr, std::ostream & out) {
	checkAbort();

	if(ptr->type != LType::relation)
        ILE("Called with arguments of the wrong type");

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
    // 1 for beginning |, 3 per field for ' | ' (space, bar, space)
	total_width += 1 + widths.size() * 3;

	const char * TL_CORNER = "┌"; // Top Left corner
	const char * TR_CORNER = "┐";
	const char * BL_CORNER = "└"; // Bottom Left corner
	const char * BR_CORNER = "┘";
	const char * M_JUNC    = "┼"; // Middle Junction
	const char * T_JUNC    = "┬";
	const char * B_JUNC    = "┴";
	const char * R_JUNC    = "┤";
	const char * L_JUNC    = "├";
	const char * HOR_BAR   = "─"; // Horizontal bar
	const char * VER_BAR   = "│";

	// print header

	// top frame
	out << TL_CORNER;
	bool first = true;
	for(auto & width : widths){
		if(first) first = false;
		else out << T_JUNC;
		for(int i = 0; i < width+2; i++) out << HOR_BAR;
	}
	out << TR_CORNER << std::endl;

	// header contents
	out << VER_BAR;
	i = 0;
	for(auto & attribute : schema->attributes){
		out << ' ' << attribute.name
			<< std::string(std::max(0, 
									(int)widths[i]
									-(int)utf8strlen(attribute.name)), ' ')
			<<  ' ' << VER_BAR;
		i++;
	}
	out << std::endl;

	// line that divides the header from the rest of the table
	out << L_JUNC;
	size_t index = 0;
	for(auto & width : widths){
		for(int i = 0; i < width+2; i++) out << HOR_BAR;
		if(index == widths.size()-1) out << R_JUNC;
		else out << M_JUNC;
		index++;
	}
	out << std::endl;

	// print body
	if(empty_schema){
		for(size_t i = 0; i < relation->tuples.size(); i++)
			out << VER_BAR << ' ' << EMPTY_MSG << ' ' << VER_BAR << std::endl;

	} else {
		checkAbort();
		for(auto & tuple : relation->tuples){
			i = 0;
			out << VER_BAR;
			for(auto & value : tuple->values){
				switch(value.type){
				case TInt:
					out << ' ' << std::right << std::setw(widths[i]);
					printIntToStream(value.intValue, out);
					break;
				case TFloat:
					out << ' ' << std::right << std::setw(widths[i]);
					printFloatToStream(value.floatValue, out);
					break;
				case TBool:
					out << ' ' << std::left << std::setw(widths[i]);
					printBoolToStream(value.boolValue, out);
					break;
				case TText:
				{
					std::string text = textToString(value.objectValue.getAs<TextBase>());
					escapeNewlines(text);
					out << ' ' << text 
						<< std::string(std::max(0, 
												(int)widths[i]
												-(int)utf8strlen(text)), ' ');
					break;
				}
				default:
					ILE("Unhandled type", value.type);
				}
				out << ' ' << VER_BAR;
				i++;
			}
			out << std::endl;
		}
	}

	// bottom frame
	out << BL_CORNER;
	first = true;
	for(auto & width : widths){
		if(first) first = false;
		else out << B_JUNC;
		for(int i = 0; i < width+2; i++) out << HOR_BAR;
	}
	out << BR_CORNER << std::endl;
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
 * Write the permutation of attributes of relation pointed to 
 * by <param>o</param> to file with path <param>name</param>
 * Assumes that <param>name</param> is already a file
 * storing the relation.
 */
void savePermutationToFile(rm_object *o, const char *name) {
	checkAbort();
	if(o->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * relation = static_cast<Relation *>(o);

	FILE *f = fopen(name, "r+");

	// advance stream to proper position
	char c;
	while ((c = fgetc(f)) != '\n') {
		if (c == '\r') {		// MAC sometimes terminates lines with only \r
			char c1 = fgetc(f);
			if (c1 != '\n') {
				ungetc(c1, f);
			}
			break;
		}
	}
	// write permutation
	for (size_t i = 0; i < relation->permutation.size(); ++i) {
		if (i != 0) {
			fputc(' ', f);
		}
		fprintf(f, "%ld", relation->permutation[i]);
	}
	fclose(f);
}

/*  Outputs The given relation to the given stream. 

	The output format is as given in the RASMUS 
	user manual:

	First the size of the schema is output, then
	the type and name of each attribute, and finally
	the values of the tuples in the relation.
 */
void saveRelationToStream(rm_object * o, std::ostream & outFile){
	checkAbort();
	if(o->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * relation = static_cast<Relation *>(o);
	outFile << relation->schema->attributes.size() << std::endl;
	
	for (size_t i = 0; i < relation->permutation.size(); ++i) {
		if (i > 0) {
			outFile << " ";
		}
		outFile << relation->permutation[i];
	}
	outFile << std::endl;
	for(auto & attribute : relation->schema->attributes){
		switch(attribute.type){
		case TInt:
			outFile << "I ";
			break;
		case TFloat:
			outFile << "F ";
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
			case TFloat:
			{
				auto lo = outFile.imbue(std::locale("C"));
				outFile << value.floatValue << std::endl;
				outFile.imbue(lo);
				break;
			}
			case TBool:
				printBoolToStream(value.boolValue, outFile);
				outFile << std::endl;
				break;
			case TText:
				printTextToStream(value.objectValue.getAs<TextBase>(), outFile);
				outFile << std::endl;
				break;
			default:
				ILE("Unhandled type", value.type);
			}
		}
		if(tuple->values.size() == 0)
			outFile << std::endl;
	}
}

/**
 * \Brief converts text to a RASMUS boolean
 */
int8_t rm_textToBool(std::string & text){
	if(text == "false")
		return RM_FALSE;
	else if(text == "?-Bool")
		return RM_NULLBOOL;
	else
		return RM_TRUE;
}

/**
 * \Brief Homemade cross platform getline function.
 */
std::istream& crossPlatformGetline(std::istream &is, std::string &str) {
	std::istream::sentry sent(is, true);
	if (!sent) {
		return is;
	}

	str.erase();
	std::stringstream ret;
	char c;
	while ((c = is.get()) != std::string::traits_type::eof()) {
		if (c == '\x00') break;							 // NULL byte break.
		if (c == '\r') {
			if (is.peek() == '\n') {
				// We have a windows formatted file.
				// Thus we should extract both before returning.
				c = is.get();
			} // otherwise we have an old mac file which terminates with only \r.
			
			break;
		}
		if (c == '\n') {
			break;
		}
		ret << c;
	}
	str = std::move(ret.str());
	return is;
}

/*  parses a relation given by an input stream and returns 
	it. The format is as given in the RASMUS user manual. 

	The attribute types and names are saved in relations->schema
	and relations->tuples contains the tuples for the relation
*/
rm_object * loadRelationFromStream(std::istream & inFile){
	checkAbort();
	size_t num_columns;
	RefPtr<Schema> schema;

	if(!(inFile >> num_columns)){
		std::string errText = 
			"The relation could not be loaded because the number "
			"of attributes could not be read from the file.";
		std::cerr << errText << std::endl;
		callback->reportError(0,0, errText);
	}

	schema = makeRef<Schema>();

	RefPtr<Relation> relations = makeRef<Relation>();
	relations->permutation.resize(num_columns, 0);
	// Read permutation.
	for (size_t i = 0; i < num_columns; ++i) {
		if (!(inFile >> relations->permutation[i])) {
			std::string errText =
				"The permutation of the schema could not be read";
			std::cerr << errText << std::endl;
			callback->reportError(0, 0, errText);
		}
	}
	
	for(size_t i = 0; i < num_columns; i++){
		char type;
		std::string name;
		Attribute attribute;

		if(!(inFile >> type >> name)){
			std::string errText =
				"The relation could not be loaded because its "
				"schema is in an unreadable format";
			std::cerr << errText << std::endl;
			callback->reportError(0, 0, errText);
		}

		attribute.name = std::move(name);

		switch(type){
		case 'T':
			attribute.type = TText;
			break;
		case 'I':
			attribute.type = TInt;
			break;
		case 'F':
			attribute.type = TFloat;
			break;
		case 'B':
			attribute.type = TBool;
			break;
		default: {
			std::string errText =
				"The relation could not be loaded because its "
				"schema is in an unreadable format";
			std::cerr << errText << std::endl;
			callback->reportError(0, 0, errText);
		}
		}

		schema->attributes.push_back(std::move(attribute));

	}

	{
		std::string _;
		crossPlatformGetline(inFile, _);
	}

	relations->schema = schema;

	bool done = false;

	// special case: either the zero or one relation
	if(schema->attributes.size() == 0){
		std::string line;
		if(crossPlatformGetline(inFile, line)){
			RefPtr<Tuple> tuple = makeRef<Tuple>();
			tuple->schema = schema;
			relations->tuples.push_back(std::move(tuple));
		}
		return relations.unbox();
	}

	while(!done){
		
		RefPtr<Tuple> tuple = makeRef<Tuple>();
		tuple->schema = schema;

		for(auto & attribute : schema->attributes){
			
			std::string line;
			if(!crossPlatformGetline(inFile, line)){
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
			case TFloat:
			{
				std::stringstream ss(line);
				ss.imbue(std::locale("C"));
				double value;
				ss >> value;
				tuple->values.emplace_back(value);
				break;
			}
			case TBool:
				tuple->values.emplace_back(rm_textToBool(line));
				break;
			case TText:
				tuple->values.emplace_back(TText, RefPtr<rm_object>::steal(rm_getConstText(line.c_str())));
				break;
			default:
				ILE("Unhandled type");
				callback->reportError(0, 0, "Unhandled type");
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
 * \Brief Prints an integer to a stream
 * Handles the special case where the integer is ?-Int
 */
void printFloatToStream(double val, std::ostream & out){
	if (std::isnan(val)) 
		out << "?-Float";
	else { 
		auto t=out.imbue(std::locale("C"));
		out << val;
		out.imbue(t);
	}
}

/**
 * \Brief prints the given tuple to the out stream
 */
void printTupleToStream(rm_object * ptr, std::ostream & out) {
	if(ptr->type != LType::tuple)
        ILE("Called with arguments of the wrong type");

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
			case TFloat:
				printFloatToStream(tup->values[i].floatValue, out);
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
 * \Brief Parses input as a CSV file
 * parseCSV returns a vector containing all the records in the CSV.
 * Each record contains a number of fields represented as strings.
 */
std::vector< std::vector<std::string> > parseCSV(std::stringstream &ss) {
	
	std::vector< std::vector<std::string> > ret;

	std::string line;
	while (crossPlatformGetline(ss, line)) {
		std::vector<std::string> row;
		std::stringstream field;
		
		bool in_quotes = false;
		for(std::vector<std::string>::size_type index = 0; index < line.size();	++index) {

			char c = line[index];

			if(!in_quotes) {
				switch(c){
				case '\x00':
					break;
				case ',':
					row.push_back(std::move(field.str()));
					field.str("");
					break;

				case '"':
					if (index > 0 && line[index-1] != ',') {
						// TODO: fixme.
						// error if not first character after a comma, or first character of line.
						std::cerr << "we should do something about this" << std::endl;
					}
					in_quotes = true;
					break;
				default:
					field << c;
				}
			} else { // inside quotes
				switch(c){
				case '\x00': {
					std::string errText =
						"Could not parse CSV file because "
						"a null byte was encountered inside quotes.";
					std::cerr << errText << std::endl;
					callback->reportError(0, 0, errText);
				}
				case '"': {
					if (index < line.size()-1) {
						char next = line[index+1];
						if (next == '"') {
							field << "\"";
						} else if (next == ',') {
							in_quotes = false;
						} else {
							std::stringstream ss;
							ss << "Could not parse CSV file because ";
							ss << "a quoted field contains data outside its quotes." << std::endl;
							ss << "Error on line " << ret.size() << std::endl
							   << "Found: '" << next << "' but expected ',' or '\"'." << std::endl;
							std::string errText = ss.str();
							std::cerr <<  errText << std::endl;
							callback->reportError(0, 0, errText);	
						}
					} else {
						in_quotes = false;
					}
					break;
				}
				default:
					// if at end of line, and inside quotes, we should read the next line and insert a line break.
					if (index < line.size()-1) {
						field << c;
					} else {
						field << std::endl;
						crossPlatformGetline(ss, line);
						// hack to start next iteration at for loop with 0,
						// since it will overflow and become 0.
						index = std::numeric_limits<std::vector<std::string>::size_type>::max();
					}

					break;
				}
			}
		}
		
		row.push_back(std::move(field.str()));
		field.str("");

		if(ret.size() > 0 && ret[0].size() != row.size()){
			std::stringstream ss;
			ss << "Could not parse CSV file because one of the records "
			   << "had a different number of fields compared to the rest"
			   << "(" << ret[0].size() << " compared to " << row.size() << ")";
			std::string errText = ss.str();
			std::cerr << errText << std::endl;
			callback->reportError(0, 0, errText);
		}
		
		ret.push_back(std::move(row));
	}

	return ret;
}

const uint8_t UNK_TYPE = 0;
const uint8_t BOOL_TYPE = 1;
const uint8_t INT_TYPE = 2;
const uint8_t TEXT_TYPE = 3;
const uint8_t FLOAT_TYPE = 4;

/**
 * \Brief returns the field-type of the given field
 * The types have the values they do because this allows 
 * us to use logical OR to set column types. E.g. if all
 * fields we have seen in a column so far are of type BOOL_TYPE
 * and we see a field of type INT_TYPE, we can OR them, and 
 * the column must be of type TEXT_TYPE.
 */
uint8_t fieldType(std::string field){
	if(field.empty()) return UNK_TYPE;
	char* p;
	strtol(field.c_str(), &p, 10);
	if(static_cast<int>(*p) == 0) // field is a valid int
		return INT_TYPE;

	if(field == "true" || field == "false" || field == "?-Bool")
		return BOOL_TYPE;
	
	std::stringstream ss(field);
	ss.imbue(std::locale("C"));
	double v;
	if (ss >> v)
		return FLOAT_TYPE;
	
	return TEXT_TYPE;
}

// partial order:
// unknown -> every type.
// int -> float.
// bool,int,float -> text.
// based on this, we should return the most specific type.
uint8_t combineTypes(uint8_t prev_type, uint8_t next_type) {
	if (prev_type == next_type) return prev_type;

	if (prev_type == UNK_TYPE && next_type != UNK_TYPE) {
		return next_type;
	}

	if ((prev_type == INT_TYPE && next_type == FLOAT_TYPE) ||
		(prev_type == FLOAT_TYPE && next_type == INT_TYPE)) {
		return FLOAT_TYPE;
	}

	if (prev_type != UNK_TYPE && next_type == UNK_TYPE) {
		return prev_type;
	}
	return TEXT_TYPE;
}

/**
 * \Brief parses the given rows, extracted from a CSV file, into a relation
 */
rm_object * parseCSVToRelation(std::vector< std::vector<std::string> > rows){
	
	RefPtr<Relation> ret = makeRef<Relation>();
	RefPtr<Schema> schema = makeRef<Schema>();
	ret->schema = schema;

	if(rows.size() == 0){
		return ret.unbox();
	}

	size_t num_fields = rows[0].size();

	ret->permutation.resize(num_fields);
	for (size_t i = 0; i < ret->permutation.size(); ++i) {
		ret->permutation[i] = i;
	}

	// determine the types of each column by examining
	// all rows except the first one (since it might be a header)
	std::vector<uint8_t> types(num_fields, UNK_TYPE);
	for(size_t i = 1; i < rows.size(); i++){
		if(rows[i].size() != num_fields)
			ILE("Unexpected number of fields while parsing CSV");
		for(size_t j = 0; j < num_fields; j++){
			std::string & field = rows[i][j];
			types[j] = combineTypes(fieldType(field), types[j]);
		}

	}

	// determine if the first row is a header or a normal row
	bool first_row_textual = true;
	bool other_rows_textual = true;
	for(size_t j = 0; j < num_fields; j++){
		uint8_t fr_type = fieldType(rows[0][j]);
		if(fr_type == BOOL_TYPE || fr_type == INT_TYPE || fr_type == FLOAT_TYPE)
			first_row_textual = false;
		if(types[j] == BOOL_TYPE || types[j] == INT_TYPE || types[j] == FLOAT_TYPE)
			other_rows_textual = false;
	}
	// note that if _all_ rows are purely textual,
	// we assume that the first row is a normal row, not special header
	bool first_row_header = (first_row_textual && !other_rows_textual);

	// fill in the schema
	for(size_t i = 0; i < num_fields; i++){
		Attribute at;
		switch(types[i]){
		case BOOL_TYPE:
			at.type = TBool;
			break;
		case INT_TYPE:
			at.type = TInt;
			break;
		case FLOAT_TYPE:
			at.type = TFloat;
			break;
		case TEXT_TYPE:
		case UNK_TYPE:
			at.type = TText;
			break;
		default:
			ILE("Unknown field type for header field number", i);
		}

		if(first_row_header)
			at.name = rows[0][i];
		else {
			std::stringstream name;
			name << "column" << i;
			at.name = name.str();
		}

		schema->attributes.push_back(std::move(at));
	}
	
	// parse the remaining rows
	for(size_t i = (first_row_header ? 1 : 0); i < rows.size(); i++){
		
		RefPtr<Tuple> tup = makeRef<Tuple>();
		tup->schema = schema;

		if(schema->attributes.size() != rows[i].size())
			// should have been caught earlier
			ILE("Row number", i, "has the wrong number of fields");

		for(size_t j = 0; j < schema->attributes.size(); j++){

			std::string & field = rows[i][j];

			switch(schema->attributes[j].type){
			case TInt:
			{
				std::stringstream iss(field);
				int64_t value;
				iss >> value;
				tup->values.emplace_back(value);
				break;
			}
			case TFloat:
			{
				std::stringstream iss(field);
				double value;
				iss >> value;
				tup->values.emplace_back(value);
				break;
			}
			case TBool:
				tup->values.emplace_back(rm_textToBool(field));
				break;
			case TText:
				tup->values.emplace_back(TText, RefPtr<rm_object>::steal(rm_getConstText(field.c_str())));
				break;
			default:
				ILE("Should not happen");
			}
		}

		ret->tuples.push_back(std::move(tup));
	}

	return ret.unbox();
}

/**
 * \Brief Loads the CSV file given by name and returns a corresponding relation
 */
rm_object * loadRelationFromCSVFile(const char * name) {
	std::ifstream ifs(name);
	std::stringstream ss;
	ss << ifs.rdbuf();
	std::vector< std::vector<std::string> > parsedCSV = parseCSV(ss);
	return parseCSVToRelation(parsedCSV);
}

/**
 * Helper method which prints field to stream
 * This method is needed for escaping characters in the field
 * as needed for the CSV format
 */
void printFieldToStream(std::string field, std::ostream & stream){

	if(field.find('"') != std::string::npos){
		std::string escaped;
		for(size_t i = 0; i < field.size(); i++){
			if(field[i] == '"')
				escaped += "\"\"";
			else
				escaped += field[i];
		}
		stream << '"' << escaped << '"';

	} else if (field.find(',') != std::string::npos ||
			   field.find("\r") != std::string::npos ||
			   field.find("\n") != std::string::npos)
		stream << '"' << field << '"';
	else
		stream << field;
}

/**
 * \Brief Saves a relation to a stream in the CSV format
 * The format conforms to RFC 4180. Lines are separated by CRLF.
 * Each field may be escaped by double quotes. In this case, double
 * quotes inside the field itself are replaced by two double quotes.
 * The column names and types of the relation are output as the first
 * line (the header) of the CSV file, with a leading I/T/B specifying
 * the column type, followed by the column name.
 */
void saveCSVRelationToStream(rm_object * o, std::ostream & stream){
	checkAbort();
	if(o->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(o);
	std::vector<size_t> &pi = rel->permutation;	

	// print the column names and types to the header of the csv file

	for (size_t i = 0; i < pi.size(); ++i) {
		if (i > 0) stream << ",";
		printFieldToStream(rel->schema->attributes[pi[i]].name, stream);
	}

	// we are not using std::endl because of RFC 4180
	stream << "\r\n"; 

	// print all the rows
	for(auto & tuple : rel->tuples) {

		for (size_t i = 0; i < pi.size(); ++i) {
			auto &val = tuple->values[pi[i]];

			if(i > 0) {
				stream << ",";
			}
			
			std::stringstream field;
			switch(val.type){
			case TInt:
				printIntToStream(val.intValue, stream);
				break;
			case TFloat:
				printFloatToStream(val.floatValue, stream);
				break;
			case TBool:
				printBoolToStream(val.boolValue, stream);
				break;
			case TText:
				printFieldToStream(textToString(val.objectValue.getAs<TextBase>()), stream);
				break;
			default:
				ILE("Unsupported type", val.type);
			}
		}
		stream << "\r\n"; 
	}
	
}

void saveCSVRelationToFile(rm_object * rel, const char * name) {
	std::ofstream file(name);
	saveCSVRelationToStream(rel, file);
}

/**
 * \Brief find the index of the column with name 'name'
 */
size_t getColumnIndex(Relation * rel, const char * name, std::pair<uint32_t, uint32_t> range){
	size_t i;
	for(i = 0; i < rel->schema->attributes.size(); i++)
		if(name == rel->schema->attributes[i].name)
			break;

	if(i == rel->schema->attributes.size()){
		std::vector<std::string> arg;
		for(size_t k = 0; k < rel->schema->attributes.size(); k++)
			arg.push_back(rel->schema->attributes[k].name);
		rm_emitColNameError(range.first, range.second, name, arg);
	}

	return i;
}

/**
 * \Brief Removes duplicate rows in a relation
 * This is done by sorting the relation's tuples, and then doing
 * a linear sweep through them to remove any duplicates
 */
void removeDuplicateRows(Relation * rel){
	checkAbort();
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

	// fix permutation.
	ret->permutation.resize(ret->schema->attributes.size());
	for (size_t i = 0 ; i < ret->permutation.size(); ++i) {
		ret->permutation[i] = i;
	}
	
	// create new tuples and add them to the new relation
	checkAbort();
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
RefPtr<Tuple> rowUnion(RefPtr<Tuple> left, RefPtr<Tuple> right, std::vector<size_t> rsi){
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
 * \Note this code is autogenerated; a maximum of 32 arguments to run() is currently supported
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
	typedef void * (*t16)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t17)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t18)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t19)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t20)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t21)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t22)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t23)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t24)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t25)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t26)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t27)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t28)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t29)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t30)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t31)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
	typedef void * (*t32)(void *, void *, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t, int64_t, int8_t);
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
	case 16: return ((t16) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel);
	case 17: return ((t17) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel);
	case 18: return ((t18) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel);
	case 19: return ((t19) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel);
	case 20: return ((t20) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel);
	case 21: return ((t21) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel);
	case 22: return ((t22) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel);
	case 23: return ((t23) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel);
	case 24: return ((t24) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel);
	case 25: return ((t25) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel, relations[24], TRel);
	case 26: return ((t26) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel, relations[24], TRel, relations[25], TRel);
	case 27: return ((t27) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel, relations[24], TRel, relations[25], TRel, relations[26], TRel);
	case 28: return ((t28) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel, relations[24], TRel, relations[25], TRel, relations[26], TRel, relations[27], TRel);
	case 29: return ((t29) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel, relations[24], TRel, relations[25], TRel, relations[26], TRel, relations[27], TRel, relations[28], TRel);
	case 30: return ((t30) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel, relations[24], TRel, relations[25], TRel, relations[26], TRel, relations[27], TRel, relations[28], TRel, relations[29], TRel);
	case 31: return ((t31) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel, relations[24], TRel, relations[25], TRel, relations[26], TRel, relations[27], TRel, relations[28], TRel, relations[29], TRel, relations[30], TRel);
	case 32: return ((t32) func)(base, ret, tup, TTup, relations[0], TRel, relations[1], TRel, relations[2], TRel, relations[3], TRel, relations[4], TRel, relations[5], TRel, relations[6], TRel, relations[7], TRel, relations[8], TRel, relations[9], TRel, relations[10], TRel, relations[11], TRel, relations[12], TRel, relations[13], TRel, relations[14], TRel, relations[15], TRel, relations[16], TRel, relations[17], TRel, relations[18], TRel, relations[19], TRel, relations[20], TRel, relations[21], TRel, relations[22], TRel, relations[23], TRel, relations[24], TRel, relations[25], TRel, relations[26], TRel, relations[27], TRel, relations[28], TRel, relations[29], TRel, relations[30], TRel, relations[31], TRel);
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

std::pair<uint32_t, uint32_t> unpackCharRange(uint64_t in){
	return std::pair<uint32_t, uint32_t>(in & 0xffffffff, in >> 32);
}


} //namespace stdlib
} //namespace rasmus

extern "C" {

// these special relations are initialized with the __init class below
Relation zero_rel;
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
	if(o->type != LType::relation)
		ILE("Called with arguments of the wrong type");
	callback->saveRelation(o, name);
}

/* wrapper function */
rm_object * rm_loadRel(const char * name) {
	return callback->loadRelation(name);
}

void rm_deleteRel(const char * name) {
	callback->deleteRelation(name);
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
rm_object * rm_joinRel(rm_object * lhs, rm_object * rhs, uint64_t range) {
	
	if(lhs->type != LType::relation || rhs->type != LType::relation)
		ILE("Called with arguments of the wrong type");

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
			   r->schema->attributes[r_index].type){
				std::stringstream ss;
				ss << "Joining requires that columns with identical names have identical types in both relations. "
				   << "The column named " << l->schema->attributes[l_index].name
				   << " has varying types " << l->schema->attributes[l_index].type 
				   << " and " << r->schema->attributes[r_index].type << ".";
				callback->reportError(unpackCharRange(range).first, 
									  unpackCharRange(range).second, ss.str());
				__builtin_unreachable();
			}
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
	
	checkAbort();
	// sort lhs and rhs, respectively, by their restricted tuples
	std::sort(l->tuples.begin(), l->tuples.end(),
			  [&](const RefPtr<Tuple> & a,
				  const RefPtr<Tuple> & b)->bool{
				  return restrict(a, lsi) <
					  restrict(b, lsi);
			  });
	
	checkAbort();
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

	// Fix permutation.
	ret->permutation.resize(ret->schema->attributes.size());
	for (size_t i = 0 ; i < ret->permutation.size(); ++i) {
		ret->permutation[i] = i;
	}

	checkAbort();
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
				RefPtr<Tuple> tup = rowUnion(l->tuples[i], r->tuples[j], rsi);
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
rm_object * rm_unionRel(rm_object * lhs, rm_object * rhs, int64_t range) {
	checkAbort();
	if(lhs->type != LType::relation || rhs->type != LType::relation)
		ILE("Called with arguments of the wrong type");

	Relation * l = static_cast<Relation *>(lhs);
	Relation * r = static_cast<Relation *>(rhs);

	// ensure schema equality
	if(l->schema->attributes.size() != r->schema->attributes.size())
		rm_emitSchemaSizeError(unpackCharRange(range).first, unpackCharRange(range).second, 
							   l->schema->attributes.size(), r->schema->attributes.size());

	// ensure the left relation is never the smaller one
	// this will make things more efficient later
	if(l->tuples.size() < r->tuples.size()){
		Relation * tmp = l;
		l = r;
		r = tmp;
	}

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
		   r->schema->attributes[right_index].name)
			rm_emitMissingColError(unpackCharRange(range).first, unpackCharRange(range).second, 
								   std::min(l->schema->attributes[left_index].name,
											r->schema->attributes[right_index].name));			
		else if (l->schema->attributes[left_index].type !=
				 r->schema->attributes[right_index].type)
			rm_emitBadColTypeError(unpackCharRange(range).first, unpackCharRange(range).second,
								   l->schema->attributes[left_index].name,
								   l->schema->attributes[left_index].type, 
								   r->schema->attributes[right_index].type);
	}
	
	// build a new relation from lhs
	RefPtr<Relation> rel = makeRef<Relation>();

	rel->schema = l->schema;
	rel->tuples = l->tuples;

	rel->permutation.resize(rel->schema->attributes.size());
	for (size_t i = 0 ; i < rel->permutation.size(); ++i) {
		rel->permutation[i] = i;
	}

	checkAbort();

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
rm_object * rm_diffRel(rm_object * lhs, rm_object * rhs, int64_t range) {
	checkAbort();
	if(lhs->type != LType::relation || rhs->type != LType::relation)
		ILE("Called with arguments of the wrong type");

	Relation * l = static_cast<Relation *>(lhs);
	Relation * r = static_cast<Relation *>(rhs);

	// ensure schema equality
	if(l->schema->attributes.size() != r->schema->attributes.size()) 
		rm_emitSchemaSizeError(unpackCharRange(range).first, unpackCharRange(range).second, 
							   l->schema->attributes.size(), r->schema->attributes.size());

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
		   r->schema->attributes[right_index].name)
			rm_emitMissingColError(unpackCharRange(range).first, unpackCharRange(range).second, 
								   std::min(l->schema->attributes[left_index].name,
											r->schema->attributes[right_index].name));			
		else if (l->schema->attributes[left_index].type !=
				 r->schema->attributes[right_index].type)
			rm_emitBadColTypeError(unpackCharRange(range).first, unpackCharRange(range).second,
								   l->schema->attributes[left_index].name,
								   l->schema->attributes[left_index].type, 
								   r->schema->attributes[right_index].type);

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

	checkAbort();
	std::sort(ls.begin(), ls.end(),
			  [](const RefPtr<Tuple> & l,
				 const RefPtr<Tuple> & r)->bool{
				  return *l < *r;
			  });
	checkAbort();
	std::sort(rs.begin(), rs.end(),
			  [](const RefPtr<Tuple> & l,
				 const RefPtr<Tuple> & r)->bool{
				  return *l < *r;
			  });

	RefPtr<Relation> ret = makeRef<Relation>();
	ret->schema = common_schema;

	ret->permutation.resize(ret->schema->attributes.size());
	for (size_t i = 0 ; i < ret->permutation.size(); ++i) {
		ret->permutation[i] = i;
	}

	checkAbort();
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

	if(rel_->type != LType::relation || func->type != LType::function)
		ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(rel_);
	FuncBase * base = (FuncBase *) func;
	selectFunc f = (selectFunc) base->func;
	AnyRet retval;

	RefPtr<Relation> ret = makeRef<Relation>();
	RefPtr<Schema> schema = makeRef<Schema>();
	schema->attributes = rel->schema->attributes;
	ret->schema = schema;

	// Fix permutation.
	ret->permutation.resize(ret->schema->attributes.size());
	for (size_t i = 0 ; i < ret->permutation.size(); ++i) {
		ret->permutation[i] = i;
	}

	checkAbort();
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
rm_object * rm_projectPlusRel(rm_object * rel_, uint32_t name_count, const char ** names, uint64_t range) {
	checkAbort();
	if(rel_->type != LType::relation)
        ILE("Called with arguments of the wrong type");

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
	inputNames.erase(
		std::unique(inputNames.begin(), inputNames.end()),
		inputNames.end());
	std::sort(schemaNames.begin(), schemaNames.end());

	if(inputNames.size() > schemaNames.size())
		rm_emitColCntError(unpackCharRange(range).first, unpackCharRange(range).second, 
						   inputNames.size(), schemaNames.size());

	std::vector<size_t> indices;
	for(size_t i = 0, j = 0; i < schemaNames.size() && j < inputNames.size(); i++){
		if(schemaNames[i].first == inputNames[j]){
			indices.push_back(schemaNames[i].second);
			j++;
		}else if(inputNames[j] < schemaNames[i].first){
			std::vector<std::string> arg;
			for(size_t k = 0; k < schemaNames.size(); k++) arg.push_back(schemaNames[k].first);
			rm_emitColNameError(unpackCharRange(range).first, unpackCharRange(range).second, inputNames[j], arg);
		}
	}

	if(indices.size() != inputNames.size()){
		std::vector<std::string> arg;
		for(size_t k = 0; k < schemaNames.size(); k++) arg.push_back(schemaNames[k].first);
		rm_emitColNameError(unpackCharRange(range).first, unpackCharRange(range).second, inputNames[inputNames.size() - 1], arg);
	}

	// project the relation onto the found indices
	RefPtr<Relation> ret = projectByIndices(rel, indices);
	removeDuplicateRows(ret.get());

	return ret.unbox();

}

/** \Brief Projects rel into all names except the given set of names
 */
rm_object * rm_projectMinusRel(rm_object * rel_, uint32_t name_count, const char ** names, uint64_t range) {
	checkAbort();
	if(rel_->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(rel_);

	// removes duplicates from inputNames and sort them
	std::vector<std::string> inputNames;
	for(uint32_t i = 0; i < name_count; i++)
		inputNames.push_back(names[i]);

	std::sort(inputNames.begin(), inputNames.end());
	inputNames.erase(
		std::unique(inputNames.begin(), inputNames.end()),
		inputNames.end());

	// prepare and sort the schema's column names
	std::vector<std::pair<std::string, size_t>> schemaNames;
	for(size_t i = 0; i < rel->schema->attributes.size(); i++)
		schemaNames.emplace_back(rel->schema->attributes[i].name, i);
	std::sort(schemaNames.begin(), schemaNames.end());

	// find the indices in the relation of the names we want to keep
	std::vector<size_t> indices;
	size_t i = 0, j = 0;
	for(; i < schemaNames.size(); i++){
		if(j < inputNames.size() && schemaNames[i].first == inputNames[j])
			j++;
		else
			indices.push_back(schemaNames[i].second);
	}

	// were all columns found?
	if(j < inputNames.size()){
		std::vector<std::string> arg;
		for(size_t k = 0; k < schemaNames.size(); k++) arg.push_back(schemaNames[k].first);
		rm_emitColNameError(unpackCharRange(range).first, unpackCharRange(range).second, inputNames[j], arg);
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
rm_object * rm_renameRel(rm_object * rel, uint32_t name_count, const char ** names, int64_t range) {
	checkAbort();
	if(rel->type != LType::relation)
        ILE("Called with arguments of the wrong type");	
	
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
		rm_emitColCntError(unpackCharRange(range).first, unpackCharRange(range).second, 
						   name_count, schema->attributes.size());

	// copy permutation of old relation
	relation->permutation.resize(relation->schema->attributes.size());
	for (size_t i = 0 ; i < old_rel->permutation.size(); ++i) {
		relation->permutation[i] = old_rel->permutation[i];
	}

	std::vector<size_t> renameMap(name_count,0);
	// linear scan to find rename mapping.
	// this is to avoid confusion with rel[A<-B, B<-A].
	for(uint32_t i = 0; i < name_count; i++){
		std::string old_name = names[i*2];
		std::string new_name = names[i*2 + 1];
		bool found = false;
		for(size_t a = 0; a < schema->attributes.size(); ++a) {
			if (schema->attributes[a].name == old_name) {
				renameMap[i] = a;
				found = true;
				break;
			}
		}
		if(!found){
			std::vector<std::string> arg;
			for(size_t k = 0; k < old_rel->schema->attributes.size(); k++)
				arg.push_back(old_rel->schema->attributes[k].name);
			rm_emitColNameError(unpackCharRange(range).first, unpackCharRange(range).second, old_name, arg);
		}
	}

	for (size_t i = 0; i < renameMap.size(); ++i) {
		std::string newName = names[2*i+1];
		schema->attributes[renameMap[i]].name = newName;
	}

	// check if we have produced a schema with duplicate names
	std::set<std::string> schema_names;
	for(auto & attribute : schema->attributes)
		if(!schema_names.insert(attribute.name).second){
			std::stringstream ss;
			ss << "The rename operation could not be performed because it would cause the column " 
			   << attribute.name << " to occur twice in the resulting relation.";
			callback->reportError(unpackCharRange(range).first, unpackCharRange(range).second, ss.str());
			__builtin_unreachable();
		}

	checkAbort();
	for(auto & tuple : relation->tuples)
		tuple->schema = schema;

	return relation.unbox();

}

/**
 * \Brief Finds the maximum value for the given column
 * \Note Ordering is not defined for text
 */
void rm_maxRel(rm_object * lhs, const char * name, AnyRet * ret, uint64_t range) {
	checkAbort();
	if(lhs->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(lhs);
	size_t index = getColumnIndex(rel, name, unpackCharRange(range));
	bool has_val = false;

	switch (rel->schema->attributes[index].type) {
	case TInt:
	{
		int64_t val = RM_NULLINT;
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].intValue;
			if (v != RM_NULLINT && (!has_val || v > val)) {
				has_val = true;
				val = v;
			}
		}
		ret->value = val;
		ret->type= TInt;
		break;
	}
	case TFloat:
	{
		union {
			double val;
			int64_t iv;
		};
		val = std::numeric_limits<double>::quiet_NaN();
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].floatValue;
			if (!std::isnan(v) && (!has_val || v > val)) {
				has_val = true;
				val = v;
			}
		}
		ret->value = iv;
		ret->type= TFloat;
		break;
	}
	case TBool:
	{
		int8_t val = RM_NULLBOOL;
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].boolValue;
			if (v != RM_NULLBOOL && (!has_val || v > val)) {
				has_val = true;
				val = v;
			}
		}
		ret->value = val;
		ret->type= TInt;
		break;
	}
	default:
		callback->reportError(unpackCharRange(range).first, unpackCharRange(range).second,
							  "max() was given a column of none numeric type");
		__builtin_unreachable();
	}
}

/**
 * \Brief Finds the minimum value for the given column
 */
void rm_minRel(rm_object * lhs, const char * name, AnyRet * ret, uint64_t range) {
	checkAbort();
	if(lhs->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(lhs);
	size_t index = getColumnIndex(rel, name, unpackCharRange(range));
	bool has_val = false;

	switch (rel->schema->attributes[index].type) {
	case TInt:
	{
		int64_t val = RM_NULLINT;
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].intValue;
			if (v != RM_NULLINT && (!has_val || v < val)) {
				has_val = true;
				val = v;
			}
		}
		ret->value = val;
		ret->type= TInt;
		break;
	}
	case TFloat:
	{
		union {
			double val;
			int64_t iv;
		};
		val = std::numeric_limits<double>::quiet_NaN();
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].floatValue;
			if (!std::isnan(v) && (!has_val || v < val)) {
				has_val = true;
				val = v;
			}
		}
		ret->value = iv;
		ret->type= TFloat;
		break;
	}
	case TBool:
	{
		int8_t val = RM_NULLBOOL;
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].boolValue;
			if (v != RM_NULLBOOL && (!has_val || v < val)) {
				has_val = true;
				val = v;
			}
		}
		ret->value = val;
		ret->type= TInt;
		break;
	}
	default:
		callback->reportError(unpackCharRange(range).first, unpackCharRange(range).second,
							  "max() was given a column of none numeric type");
		__builtin_unreachable();
	}
}

/**
 * \Brief Returns the sum of all values for the given column
 */
void rm_addRel(rm_object * lhs, const char * name, AnyRet * ret, uint64_t range) {
	checkAbort();
	if(lhs->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(lhs);
	size_t index = getColumnIndex(rel, name, unpackCharRange(range));

	switch (rel->schema->attributes[index].type) {
	case TInt:
	{
		int64_t sum = 0;
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].intValue;
			if (v != RM_NULLINT) sum += v; 
		}
		ret->value = sum;
		ret->type = TInt;
		break;
	}
	case TFloat:
	{
		union {
			double sum;
			int64_t iv;
		};
		sum=0.0;
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].floatValue;
			if (!std::isnan(v)) sum += v; 
		}
		ret->value = iv;
		ret->type = TFloat;
		break;
	}
	default:
		rm_emitBadCalcTypeError(unpackCharRange(range).first, unpackCharRange(range).second,
								name, PlainType(rel->schema->attributes[index].type), "addition");
		break;
	}
}

/**
 * \Brief Returns the product of all values for the given column
 */
void rm_multRel(rm_object * lhs, const char * name, AnyRet * ret, uint64_t range) {
	checkAbort();
	if(lhs->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(lhs);
	size_t index = getColumnIndex(rel, name, unpackCharRange(range));

	switch (rel->schema->attributes[index].type) {
	case TInt:
	{
		int64_t product = 1;
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].intValue;
			if (v != RM_NULLINT) product *= v; 
		}
		ret->value = product;
		ret->type = TInt;
		break;
	}
	case TFloat:
	{
		union {
			double product;
			int64_t iv;
		};
		product=1.0;
		for(auto tup : rel->tuples) {
			auto v=tup->values[index].floatValue;
			if (!std::isnan(v)) product *= v; 
		}
		ret->value = iv;
		ret->type = TFloat;
		break;
	}
	default:
		rm_emitBadCalcTypeError(unpackCharRange(range).first, unpackCharRange(range).second,
								name, PlainType(rel->schema->attributes[index].type), "Multiplication");
		break;
	}
}

/**
 * \Brief Return the number of non-null entries in the given column
 */
int64_t rm_countRel(rm_object * lhs, const char * name, uint64_t range) {
	checkAbort();
	if(lhs->type != LType::relation)
        ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(lhs);

	size_t index = getColumnIndex(rel, name, unpackCharRange(range));
	int64_t count = 0;

	switch(rel->schema->attributes[index].type){
	case TInt:
		for(auto tup : rel->tuples)
			if(tup->values[index].intValue != RM_NULLINT)
				count++;
		break;
	case TFloat:
		for(auto tup : rel->tuples)
			if(!std::isnan(tup->values[index].intValue))
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
rm_object * rm_createTup(uint32_t count, TupEntry * entries, int64_t range) {
	RefPtr<Tuple> t = makeRef<Tuple>();
	
	RefPtr<Schema> schema = makeRef<Schema>();
	t->schema = schema;
	
	for(uint32_t i = 0; i < count; i++){
		// add entry to the tuple's schema 
		TupEntry te = entries[i];
		Attribute attribute {
			(PlainType) te.type, te.name
		};
		schema->attributes.push_back(attribute);
		
		// add entry to the tuple's values
		switch(te.type){
		case TInt:
			t->values.emplace_back(te.value);
			break;
		case TFloat: {
			union {
				double fpv;
				int64_t iv;
			};
			iv=te.value;
			t->values.emplace_back(fpv);
			break;
		}
		case TBool:
			t->values.emplace_back(static_cast<int8_t>(te.value));
			break;
		case TText: 
		{
			TextBase * text = reinterpret_cast<TextBase *>(te.value);
			RefPtr<rm_object> ref(text);
			t->values.emplace_back((PlainType) te.type, std::move(ref));
			break;
		}
		default:
			ILE("bad object type while creating tuple", te.type);
		}
	}

	// check if we have produced a schema with duplicate names
	std::set<std::string> schema_names;
	for(auto & attribute : schema->attributes)
		if(!schema_names.insert(attribute.name).second){
			std::stringstream ss;
			ss << "The tuple could not be created because it would cause the column "
			   << attribute.name << " to occur twice.";
			callback->reportError(unpackCharRange(range).first, unpackCharRange(range).second, ss.str());
			__builtin_unreachable();
		}

	return t.unbox();
}

/**
 * \Brief Creates the relation containing a single tuple
 */
rm_object * rm_createRel(rm_object * tup) {

	if(tup->type != LType::tuple)
        ILE("Called with arguments of the wrong type");

	Tuple * tuple = static_cast<Tuple *>(tup);

	if(!tuple) ILE("Null tuple in rm_createRel");
	if(!tuple->schema) ILE("Tuple has null schema in rm_createRel");

	RefPtr<Relation> rel = makeRef<Relation>();

	rel->schema = tuple->schema;
	rel->tuples.push_back(RefPtr<Tuple>(tuple));

	rel->permutation.resize(tuple->schema->attributes.size());
	for (size_t i = 0; i < rel->permutation.size(); ++i) {
		rel->permutation[i] = i;
	}
	
	return rel.unbox();
}

/**
 * \Brief Fetch the value given by name from tup
 * \Note the value is saved in ret
 */
void rm_tupEntry(rm_object * tup, const char * name, AnyRet * ret, int64_t range) {

	if(tup->type != LType::tuple)
        ILE("Called with arguments of the wrong type");

	Tuple * tuple = static_cast<Tuple *>(tup);
	Schema * schema = tuple->schema.get();

	size_t i;
	for(i = 0; i < schema->attributes.size(); i++)
		if(schema->attributes[i].name == name)
			break;

	if(i == schema->attributes.size()){
		std::vector<std::string> arg;
		for(size_t k = 0; k < schema->attributes.size(); k++)
			arg.push_back(schema->attributes[k].name);
		rm_emitTupColNameError(unpackCharRange(range).first, unpackCharRange(range).second, name, arg);
	}
	
	AnyValue val = tuple->values[i];
	ret->type = val.type;
	switch(val.type){
	case TInt:
		ret->value = val.intValue;
		break;
	case TFloat:
		ret->value = val.floatValue;
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
 * \Note Shared values and types are taken from rhs, not lhs
 */
rm_object * rm_extendTup(rm_object * lhs_, rm_object * rhs_) {

	if(lhs_->type != LType::tuple || rhs_->type != LType::tuple)
		ILE("Called with arguments of the wrong type");

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
rm_object * rm_tupRemove(rm_object * tup, const char * name, int64_t range) {

	if(tup->type != LType::tuple)
        ILE("Called with arguments of the wrong type");

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

	if(!found_name){
		std::vector<std::string> arg;
		for(size_t k = 0; k < old_schema->attributes.size(); k++)
			arg.push_back(old_schema->attributes[k].name);
		rm_emitTupColNameError(unpackCharRange(range).first, unpackCharRange(range).second, name, arg);
	}

	return new_tup.unbox();
}

/**
 * \Brief Checks whether or not the given tuple's schema contains the given name
 */
uint8_t rm_tupHasEntry(rm_object * tup, const char * name) {

	if(tup->type != LType::tuple)
		ILE("Called with arguments of the wrong type");

	Tuple * tuple = static_cast<Tuple *>(tup);

	for(auto attribute : tuple->schema->attributes){
		if(name == attribute.name) return RM_TRUE;
	}

	return RM_FALSE;
}

/**
 * \Brief Checks whether the tuple's column with name 'name' has type 'type'
 */
uint8_t rm_tupEntryType(rm_object * _tup, const char * name, PlainType type, int64_t range ) {
	if(_tup->type != LType::tuple)
		ILE("Called with arguments of the wrong type");

	Tuple * tup = static_cast<Tuple *>(_tup);
	
	for(auto & attribute : tup->schema->attributes)
		if(name == attribute.name)
			return type == attribute.type ? RM_TRUE : RM_FALSE;

	// if we get here, the column was not found
	std::vector<std::string> arg;
	for(auto & attribute : tup->schema->attributes)
		arg.push_back(attribute.name);
	rm_emitTupColNameError(unpackCharRange(range).first, unpackCharRange(range).second, name, arg);
	
}

/**
 * \Brief Checks whether or not the given relation's schema contains the given name
 */
uint8_t rm_relHasEntry(rm_object * rel, const char * name) {

	if(rel->type != LType::relation)
		ILE("Called with arguments of the wrong type");

	Relation * relation = static_cast<Relation *>(rel);
	if(!relation) ILE("Null relation passed to rm_relHasEntry");
	if(!relation->schema) ILE("Null schema for relation passed to rm_relHasEntry");

	for(auto attribute : relation->schema->attributes){
		if(name == attribute.name) return RM_TRUE; 
	}

	return RM_FALSE;
}

/**
 * \Brief Checks whether the column in 'rel' with name 'name' has type 'type'
 */
uint8_t rm_relEntryType(rm_object * rel, const char * name, int8_t type, int64_t range) {

	if(rel->type != LType::relation)
		ILE("Called with arguments of the wrong type");

	Relation * relation = static_cast<Relation *>(rel);
	if(!relation) ILE("Null relation passed to rm_relHasEntry");
	if(!relation->schema) ILE("Null schema for relation passed to rm_relHasEntry");

	for(auto & attribute : relation->schema->attributes)
		if(name == attribute.name) 
			return type == attribute.type ? RM_TRUE : RM_FALSE;

	// if we get here, the column 'name' was not found
	std::vector<std::string> arg;
	for(auto & attribute : relation->schema->attributes)
		arg.push_back(attribute.name);
	rm_emitColNameError(unpackCharRange(range).first, unpackCharRange(range).second, name, arg);
}

/**
 * \Brief Checks if lhs and ehs are identical
 * They must be completely identical; same schema, same number of tuples etc.
 */
uint8_t rm_equalRel(rm_object * lhs, rm_object * rhs) {
	checkAbort();
	if(lhs->type != LType::relation || rhs->type != LType::relation)
		ILE("Called with arguments of the wrong type");

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

	checkAbort();
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

	checkAbort();
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

	checkAbort();
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

	if(lhs->type != LType::tuple || rhs->type != LType::tuple)
        ILE("Called with arguments of the wrong type");

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
 * \Brief Sorts the relation rel_ by the column given by col_num
 * If ascending is false, the resulting column has descending order
 */
void rm_sortRel(rm_object * rel_, size_t col_num, bool ascending){

	if(rel_->type != LType::relation)
		ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(rel_);

	if(col_num >= rel->schema->attributes.size())
		ILE("The column number to sort by was out of range");

	if(ascending)
		std::stable_sort(rel->tuples.begin(), rel->tuples.end(),						 
						 [&](const RefPtr<Tuple> & a,
							 const RefPtr<Tuple> & b)->bool{
							 return a->values[col_num] < b->values[col_num];
						 });		
	else 
		std::stable_sort(rel->tuples.begin(), rel->tuples.end(),						 
						 [&](const RefPtr<Tuple> & a,
							 const RefPtr<Tuple> & b)->bool{
							 return b->values[col_num] < a->values[col_num];
						 });
}

/**
 * \Brief runs 'func' on each tuple in 'rel_' and returns a relation which is the union of the results
 */
rm_object * rm_forAll(rm_object * rel_, rm_object * func, int64_t range){
	checkAbort();
	if(rel_->type != LType::relation || func->type != LType::function)
        ILE("Called with arguments of the wrong type");

	Relation * rel = static_cast<Relation *>(rel_);
	RefPtr<Relation> ret = makeRef<Relation>();
	ret->schema = makeRef<Schema>();

	RefPtr<Relation> arg_rel = makeRef<Relation>();
	arg_rel->schema = makeRef<Schema>();
	
	bool ret_initialized = false;
	
	checkAbort();
	for(size_t i = 0; i < rel->tuples.size(); i++){
		Tuple * tup = rel->tuples[i].get();
		int64_t arg_tup = reinterpret_cast<int64_t>(tup);
		int64_t arg_rel_as_int = reinterpret_cast<int64_t>(arg_rel.get());

		AnyRet retval;
		FuncBase * base = (FuncBase *) func;
		typedef void * (*t0)(FuncBase *, AnyRet *, int64_t, int8_t, int64_t, int8_t);

		((t0) base->func)(base, &retval, arg_tup, TTup, arg_rel_as_int, TRel);

		if (retval.type != TRel) ILE("Should not happen");
		Relation * _result = reinterpret_cast<Relation *>(retval.value);

		RefPtr<Relation> result = RefPtr<Relation>::steal(_result);

		if (!ret_initialized){
			ret->schema->attributes = result->schema->attributes;
			ret->permutation.resize(ret->schema->attributes.size());
			ret_initialized = true;
			for (size_t i = 0; i < ret->schema->attributes.size(); ++i) {
				ret->permutation[i] = i;
			}
		} else if(!schemaEquals(ret->schema.get(), result->schema.get()))
			rm_emitFuncDiffSchemasError(unpackCharRange(range).first, unpackCharRange(range).second, "forall");

		for (auto & tuple : result->tuples)
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
rm_object * rm_factorRel(uint32_t num_col_names, char ** col_names, uint32_t num_relations, rm_object ** relations, rm_object * func, int64_t range){
	checkAbort();
	// preliminary checks

	// when factor is given no columns, it should implicitly use all of them;
	// this is equivalent to a forall
	if(num_col_names == 0){
		if(num_relations != 1){
			std::stringstream ss;
			ss << "The forall operator can only be used on exactly one relation, but "
			   << num_relations << " were given.";
			callback->reportError(unpackCharRange(range).first, unpackCharRange(range).second, ss.str());
			__builtin_unreachable();
		}

		if(relations[0]->type != LType::relation)
			ILE("Called with arguments of the wrong type");

		return rm_forAll(relations[0], func, range);
	}

	if(num_relations < 1){
		callback->reportError(unpackCharRange(range).first, unpackCharRange(range).second, 
							  "No relations were given to factor.");
		__builtin_unreachable();
	}
	
	if(num_relations > 32){
		callback->reportError(unpackCharRange(range).first, unpackCharRange(range).second, 
							  "RASMUS does not currently support factoring more than 32 relations at once.");
		__builtin_unreachable();
	}

	for(size_t i = 0; i < num_relations; i++)
		if(relations[i]->type != LType::relation)
			ILE("Called with arguments of the wrong type");

	if(func->type != LType::function)
		ILE("Called with arguments of the wrong type");

	std::vector<std::string> names;
	for(uint32_t i = 0; i < num_col_names; i++)
		names.push_back(col_names[i]);

	std::sort(names.begin(), names.end());
	auto idx = std::adjacent_find(names.begin(), names.end());
	if(idx != names.end()){
		std::stringstream ss;
		ss << "The column with name " << *idx << " was provided more than once to factor.";
		callback->reportError(unpackCharRange(range).first, unpackCharRange(range).second, ss.str());
		__builtin_unreachable();
	}
		
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

		if(rel_indices[i].size() != names.size()){
			if(rel_indices[i].size() > names.size()) ILE("Should not happen");
			std::string missing_col_name = names[rel_indices[i].size()];
			std::vector<std::string> arg;
			for(size_t k = 0; k < cur_rel->schema->attributes.size(); k++)
				arg.push_back(cur_rel->schema->attributes[k].name);
			rm_factorMissingColError(unpackCharRange(range).first, unpackCharRange(range).second,
									 missing_col_name, i, arg);
		}
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
			   rel2->schema->attributes[index2].type){
				std::stringstream ss;
				ss << "Factoring requires each column to have identical types in each relation. "
				   << "The column named " << rel1->schema->attributes[index1].name
				   << " has varying types " << rel1->schema->attributes[index1].type 
				   << " and " << rel2->schema->attributes[index2].type 
				   << " in relations number 0 and " << i << ".";
				callback->reportError(unpackCharRange(range).first, 
									  unpackCharRange(range).second, ss.str());
				__builtin_unreachable();
			}
		}
	}
	
	// sort all the relations on their restricted tuples
	checkAbort();
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

	checkAbort();
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
			ret->permutation.resize(ret->schema->attributes.size());
			for (size_t i = 0; i < ret->permutation.size(); ++i) {
				ret->permutation[i] = i;
			}
			ret_initialized = true;
		}else if(!schemaEquals(ret->schema.get(), result->schema.get()))
			rm_emitFuncDiffSchemasError(unpackCharRange(range).first, unpackCharRange(range).second, "factor");

		for(auto & tuple : result->tuples)
			ret->tuples.push_back(tuple);
	}

	// TODO Q: if there are no rows to process, we return the zero relation. Is this correct?

	removeDuplicateRows(ret.get());	
	return ret.unbox();
}

} // extern "C"
