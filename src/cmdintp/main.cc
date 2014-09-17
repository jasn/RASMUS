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
#include <frontend/interpreter.hh>
#include <stdlib/lib.h>
#include <iostream>
#include <fstream>
#include "promptLine.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void usage(std::ostream & out) {
	out << "Usage: rm [OPTIONS] [FILE]" << std::endl
		<< "Run rasmus interpreter, if FILE is given run on that file" << std::endl
		<< "otherwise read commands from stdin" << std::endl
		<< std::endl
		<< "Options:" << std::endl
		<< "  -d, --dump                Short for all --dump-* commands" << std::endl
		<< "      --dump-raw-function   Dump raw llvm-ir for functions" << std::endl
		<< "      --dump-opt-function   Dump optimized llvm-ir for functions" << std::endl
		<< "      --dump-ast            Dump the ast" << std::endl
		<< "  -h, --help                Display this help" << std::endl
		<< "  -v, --version             Display version information" << std::endl;
}

void version(std::ostream & out) {
	out << "rm (pyRASMUS) 0.1" << std::endl
		<< "Copyright (C) 2014 pyRASMUS authors" << std::endl
		<< "License LGPLv3+: GNU LGPL version 3 or later <http://gnu.org/licenses/lgpl.html>." << std::endl 
		<< "This is free software: you are free to change and redistribute it." << std::endl
		<< "There is NO WARRANTY, to the extent permitted by law." << std::endl
		<< std::endl
		<< "Written by Thomas Engelbrecht Hybel, Jesper Asbjørn Sindahl Nielsen and Jakob Truelsen" << std::endl;
}

void abortComputation(int) {
	rm_abort();
}

int main(int /*argc*/, char ** argv) {
	bool displayUsage=false;
	bool displayVersion=false;
	int options=0;
	bool hasDashDash=false;
	std::string file;
	char ** argp=argv;
	++argp;
	while (*argp) {
		char *arg=*argp;
		if (!hasDashDash && arg[0] == '-') {
			if (arg[1] == '-') {
				if (!strcmp(arg,"--")) 
					hasDashDash=true;
				else if (!strcmp(arg, "--help"))
					displayUsage=true;
				else if (!strcmp(arg, "--version"))
					displayVersion=true;
				else if (!strcmp(arg, "--dump")) 
					options |= 
						rasmus::frontend::Interperter::DumpRawFunction |
						rasmus::frontend::Interperter::DumpOptFunction |
						rasmus::frontend::Interperter::DumpAST |
						rasmus::frontend::Interperter::DumpAllocations;
				else if (!strcmp(arg, "--dump-raw-function"))
					options |= rasmus::frontend::Interperter::DumpRawFunction;
				else if (!strcmp(arg, "--dump-opt-function"))
					options |= rasmus::frontend::Interperter::DumpOptFunction;
				else if (!strcmp(arg, "--dump-allocations"))
					options |= rasmus::frontend::Interperter::DumpAllocations;
				else if (!strcmp(arg, "--dump-ast"))
					options |= rasmus::frontend::Interperter::DumpAST;
				else {
					std::cerr << "Unknown switch " << arg << std::endl;
					usage(std::cerr);
					exit(EXIT_FAILURE);
				}
			} else {
				char *c=arg+1;
				while (*c) {
					switch(*c) {
					case 'h':
						displayUsage=true;
						break;
					case 'v':
						displayVersion=true;
						break;
					case 'd':
						options |= 
							rasmus::frontend::Interperter::DumpRawFunction |
							rasmus::frontend::Interperter::DumpOptFunction |
							rasmus::frontend::Interperter::DumpAST |
							rasmus::frontend::Interperter::DumpAllocations;
						break;
					default:
						std::cerr << "Unknown switch " << *c << std::endl;
						usage(std::cerr);
						exit(EXIT_FAILURE);
					}
					++c;
				}
			}
		} else {
			if (!file.empty()) {
				std::cerr << "rasmus can only run one file at a time" << std::endl;
				usage(std::cerr);
				exit(EXIT_FAILURE);
			}
			file=arg;
		}
		++argp;
	}

	if (displayUsage) {
		usage(std::cout);
		exit(EXIT_SUCCESS);
	}

	if (displayVersion) {
		version(std::cout);
		exit(EXIT_SUCCESS);
	}

	std::shared_ptr<rasmus::frontend::Callback> callback = std::make_shared<rasmus::frontend::TerminalCallback>();
	std::shared_ptr<rasmus::frontend::Interperter> interperter=rasmus::frontend::makeInterperter(callback);
	interperter->setup(options, file.empty()?std::string("Interpreter"):file);
	if (!file.empty()) {
		std::ifstream is(file);
		if (is.fail()) {
			std::cerr << "Unable to open file \"" << file << "\"" << std::endl;
			exit(EXIT_FAILURE);
		}
		is.seekg(0, std::ios_base::end);
		std::streampos fileSize = is.tellg();
		is.seekg(0, std::ios_base::beg);
		std::string s(fileSize, ' ');
		is.read(&s[0], fileSize);
		if (interperter->runLine(s))
			exit(EXIT_SUCCESS);
		else
			exit(EXIT_FAILURE);
	}

	signal(SIGINT, SIG_IGN);
	while (true) {
		std::string line;
		PromptStatus s = promptLine(interperter->complete(), line);
		bool done=false;
		switch (s) {
		case PromptStatus::NORMAL:
			signal(SIGINT, abortComputation);
			interperter->runLine(line);
			signal(SIGINT, SIG_IGN);
			rm_clearAbort();
			break;
		case PromptStatus::CANCEL:
			if (interperter->complete())
				printf("\nPress CTRL+D to stop rasmus\n");
			else 
				printf("\n");
			interperter->cancel();
			break;
		case PromptStatus::EMPTY:
			break;
		case PromptStatus::DONE:
			done=true;
			break;
		}
		if (done) break;
	}
	printf("\n");
	exit(EXIT_SUCCESS);
}
